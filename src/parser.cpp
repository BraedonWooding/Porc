#include "parser.hpp"

#include <rang.hpp>

#include "helper.hpp"

namespace porc {

/*
  As to make sure that we give much nicer errors we split our errors up like this;
  - tokenizer errors (for example 3e + 2)
  - syntatical errors (for example forgetting semicolons)
  - semantic/Syntax errors (invalid types or whatever)

  It will stop at building the AST if there are any tokenizer/syntatical errors
  But will still try to build the AST it gets around this by basically skipping
  any problematic expressions, since we don't care about producing code for this
  and just to report the errors we don't have to care about producing an
  accurate AST (it is going to be wrong regardless).
*/

// @TODO: add consumeToken but using missing since sometimes we want that

bool Parser::ConsumeToken(Token::Kind wanted) {
  Token tok = stream.PeekCur();
  // check if next token is what we want
  if (tok.type == Token::EndOfFile) {
    err.ReportMissingToken(wanted, tok.pos);
    return false;
  } else if (tok.type != wanted) {
    err.ReportUnexpectedToken(wanted, tok);
    return false;
  } else {
    stream.PopCur(); // actually consume token
    return true;
  }
}

template<typename T>
LineRange GetLineRangeForVec(vector_unique_ptr<T> &vec, int index) {
  return vec.at(index)->pos;
}

template<typename T>
LineRange GetLineRangeForVec(std::vector<T> &vec, int index) {
  return vec.at(index).pos;
}

template<typename T>
LineRange FindRangeOfVector(std::vector<T> &vec) {
  switch (vec.size()) {
    // @TODO: What do we do?
    //        do we want to be able to find the range??
    case 0: Unreachable("FindRangeOfVector of an empty vector");
    case 1: return GetLineRangeForVec(vec, 0);
    default: return LineRange(GetLineRangeForVec(vec, 0),
                              GetLineRangeForVec(vec, vec.size() - 1));
  }
}

template<typename T, typename In, typename... Args>
optional_unique_ptr<T> ComposeExpr(optional_unique_ptr<In> in, Args... args) {
  if (in) {
    auto expr = std::move(*in);
    return std::make_unique<T>(expr->pos, std::move(expr),
                               std::move(args)...);
  } else {
    return std::nullopt;
  }
}

template<typename To>
std::optional<To> Parser::TokenCast(Token tok) {
  std::optional<To> cast = To::FromToken(tok);
  if (!cast) {
    err.ReportInvalidTokenCast(tok, To::AllMsg());
  }
  return cast;
}

optional_unique_ptr<FileDecl> Parser::ParseFileDecl() {
  vector_unique_ptr<FuncStatement> exprs;
  vector_unique_ptr<TypeDecl> types;
  Token tok;

  while ((tok = stream.PeekCur()).type != Token::EndOfFile) {
    if (tok.type == Token::Type) {
      if (auto type = ParseTypeDecl()) {
        types.push_back(std::move(*type));
      } else {
        // @TODO: recover
        return std::nullopt;
      }
    } else {
      if (auto expr = ParseFuncStatement(true)) {
        exprs.push_back(std::move(*expr));
      } else {
        // @TODO: recover
        return std::nullopt;
      }
    }
  }

  LineRange range = stream.PeekCur().pos;
  LineRange exprs_range = range;
  LineRange type_range = range;
  if (exprs.size() > 0) exprs_range = FindRangeOfVector(exprs);
  if (types.size() > 0) type_range = FindRangeOfVector(types);
  // technically this is a bit of an unorthodox way of doing it
  // but since the line range takes the minimum and maximum we don't
  // have to manually verify which one is which.
  range = LineRange(exprs_range, type_range);
  return std::make_unique<FileDecl>(range, std::move(exprs), std::move(types));
}

FuncStatement::PrefixKind Parser::ParseFuncStatementPrefix() {
  u8 ret = FuncStatement::NoPrefix;
  if (stream.PeekCur().type == Token::Yield) {
    stream.PopCur();
    ret |= FuncStatement::Yield;
  }

  switch (stream.PeekCur().type) {
    case Token::Return: {
      stream.PopCur();
      ret |= FuncStatement::Return;
    } break;
    case Token::Continue: {
      stream.PopCur();
      ret |= FuncStatement::Continue;
    } break;
    case Token::Break: {
      stream.PopCur();
      ret |= FuncStatement::Break;
    } break;
    case Token::Assign: {
      stream.PopCur();
      ret |= FuncStatement::BlockVal;
    } break;
    default: break;
  }

  return static_cast<FuncStatement::PrefixKind>(ret);
}

// @TODO: choose better name than force_terminator_if_req
optional_unique_ptr<FuncStatement> Parser::ParseFuncStatement(bool file_scope,
      bool force_terminator_if_req) {
  Token tok = stream.PeekCur();
  LineRange start = tok.pos;
  optional_unique_ptr<FuncStatement> expr = std::nullopt;

  bool is_block_decl = false;
  FuncStatement::PrefixKind ret = ParseFuncStatementPrefix();
  if (file_scope && ret != FuncStatement::NoPrefix) {
    err.ReportCustomErr("Can't yield, return, continue, evaluate, or break"
                        " from file scope", tok.pos, ErrStream::SemanticErr);
    return std::nullopt;
  }

  std::vector<LineStr> lhs_str = ParseIds(Token::Comma);
  bool is_expr = false;
  tok = stream.PeekCur();

  if (tok.type == Token::Colon || tok.type == Token::DoubleColon ||
      tok.type == Token::ColonAssign) {
    // is a var decl
    // turn the line strings into declarations
    std::vector<VarDecl::Declaration> decls;
    for (auto &id : lhs_str) {
      decls.push_back(VarDecl::Declaration(id, std::nullopt, std::nullopt));
    }
    expr = ComposeExpr<FuncStatement>(ParseRhsVarDecl(std::move(decls)));
  } else if (tok.IsAssignmentOp()) {
    expr = ComposeExpr<FuncStatement>(ParseAssignmentExprVarDeclHint(
      std::move(lhs_str)));
  } else {
    // has to be an expr
    if (lhs_str.size() > 1) {
      err.ReportCustomErr("Can't apply comma operator to expressions",
                          tok.pos, ErrStream::SyntaxErr);
      return std::nullopt;
    } else if (lhs_str.size() == 1) {
      stream.Push(Token(Token::Identifier, lhs_str[0].pos,
                        static_cast<std::string>(lhs_str[0])));
    }
    is_expr = true;
    expr = ComposeExpr<FuncStatement>(ParseExpr(), ret);
  }

  if (!expr) return std::nullopt;

  Token last = stream.LastPopped();
  // if the last token passed was a right brace we don't need a line terminator
  bool requires_terminator = last.type != Token::RightBrace &&
                             force_terminator_if_req;

  if (requires_terminator) {
    // Slightly better error msg
    if (stream.PeekCur().type != Token::SemiColon) {
      err.ReportMissingToken(Token::SemiColon, last.pos);
      return std::nullopt;
    }
    stream.PopCur();
  }

  if (!is_expr && ret != FuncStatement::NoPrefix) {
    err.ReportCustomErr("Can't yield, return, continue, evaluate, or break"
                        " a non expr", tok.pos, ErrStream::SemanticErr);
    return std::nullopt;
  }
  return expr;
}

optional_unique_ptr<TypeDecl> Parser::ParseTypeDecl() {
  LineRange start = stream.PeekCur().pos;
  if (!ConsumeToken(Token::Type)) return std::nullopt;

  Token tok = stream.PopCur();
  if (tok.type != Token::Identifier) {
    err.ReportUnexpectedToken(Token::Identifier, tok);
    return std::nullopt;
  }
  auto id = *tok.ToLineStr();
  optional_unique_ptr<TypeExpr> type = std::nullopt;
  vector_unique_ptr<TypeStatement> block;

  tok = stream.PeekCur();
  if (tok.type == Token::Is) {
    stream.PopCur();
    if (auto tmp = ParseTypeExpr()) {
      type = std::move(*tmp);
    } else {
      return std::nullopt;
    }
  }

  tok = stream.PeekCur();
  LineRange end = stream.PeekCur().pos;

  if (tok.type == Token::LeftBrace) {
    optional_unique_ptr<TypeStatement> expr;
    Token tok;
    while ((tok = stream.PeekCur()).type != Token::RightBrace &&
          tok.type != Token::EndOfFile && (expr = ParseTypeStatement())) {
      block.push_back(std::move(*expr));
    }
    if (!expr) return std::nullopt;
    end = FindRangeOfVector(block);
    if (!ConsumeToken((Token::RightBrace))) return std::nullopt;
  } else if (!type) {
    // we have neither a type nor a block this is invalid
    err.ReportCustomErr("Was expecting either a type block or a type expr "
                        "got neither.", tok.pos, ErrStream::SyntaxErr);
  }

  LineRange pos = LineRange(start, end);
  tok = stream.PeekCur();
  if (tok.type == Token::SemiColon || block.size() == 0) {
    if (!ConsumeToken(Token::SemiColon)) {
      return std::nullopt;
    }
  }
  return std::make_unique<TypeDecl>(pos, std::move(id),
                                    std::move(type), std::move(block));
}

// optional_unique_ptr<VarDecl> Parser::ParseFuncDecl() {
//   if (!ConsumeToken(Token::Func)) return std::nullopt;

//   Token tok = stream.PopCur();
//   if (tok.type != Token::Identifier) {
//     err.ReportUnexpectedToken(Token::Identifier, tok);
//     return std::nullopt;
//   }

//   auto id = *tok.ToLineStr();
//   auto tuple_decl = ParseTupleValueDecl();
//   if (!tuple_decl) return std::nullopt;

//   optional_unique_ptr<TypeExpr> ret_type = std::nullopt;
//   if (stream.PeekCur().type != Token::LeftBrace) {
//     if (!ConsumeToken(Token::ReturnType)) return std::nullopt;
//     auto type_expr = ParseTypeExpr();
//     if (!type_expr) return std::nullopt;
//     ret_type = std::move(*type_expr);
//   }

//   auto expr = ParseBlock();
//   if (!expr) return std::nullopt;
//   LineRange pos = LineRange((*tuple_decl)->pos, (*expr)[expr->size()-1]->pos);

//   auto tmp = std::make_unique<Expr>(pos, std::move(*tuple_decl), 
//                                     std::move(ret_type),
//                                     std::move(*expr));
//   std::vector<VarDecl::Declaration> decl;
//   decl.push_back(VarDecl::Declaration(id, std::nullopt, std::move(tmp)));
//   return std::make_unique<VarDecl>(pos, false, std::move(decl));
// }

/*
  The following is ugly ish
  It is mostly due to me having to do a bad conversion
  This is kinda rare and fine, a slightly better way could be
  to have this kind of conversion inside the AST that is allow the ast
  to have another option that jumps from expr -> postfix

  Also note: that in this case it is doing the same conversion really
*/

std::unique_ptr<Expr> Parser::ConvIdentToExpr(LineStr id) {
  LineRange pos = id.pos;
  return std::make_unique<Expr>(pos,
         std::make_unique<LogicalOrExpr>(pos,
         std::make_unique<LogicalAndExpr>(pos,
         std::make_unique<ComparisonExpr>(pos,
         std::make_unique<AdditiveExpr>(pos,
         std::make_unique<MultiplicativeExpr>(pos,
         std::make_unique<UnaryExpr>(pos,
         std::make_unique<PowerExpr>(pos,
         std::make_unique<Atom>(pos, id)))))))));
}

std::unique_ptr<AdditiveExpr> Parser::ExprToFold(std::unique_ptr<AdditiveExpr> expr,
                                         bool folding, LineRange pos,
                                         std::unique_ptr<Atom> func) {
  return std::make_unique<AdditiveExpr>(pos,
         std::make_unique<MultiplicativeExpr>(pos,
         std::make_unique<UnaryExpr>(pos,
         std::make_unique<PowerExpr>(pos,
         std::make_unique<Atom>(pos, std::move(func), folding,
                                std::move(expr))))));
}

std::unique_ptr<Expr> Parser::ParenthesiseExpr(std::unique_ptr<Expr> expr) {
  LineRange pos = expr->pos;
  return std::make_unique<Expr>(pos,
         std::make_unique<LogicalOrExpr>(pos,
         std::make_unique<LogicalAndExpr>(pos,
         std::make_unique<ComparisonExpr>(pos,
         std::make_unique<AdditiveExpr>(pos,
         std::make_unique<MultiplicativeExpr>(pos,
         std::make_unique<UnaryExpr>(pos,
         std::make_unique<PowerExpr>(pos,
         std::make_unique<Atom>(pos, std::move(expr))))))))));
}

optional_unique_ptr<IdentifierAccess>
    Parser::ParseIdentifierAccess(Token::Kind continuer) {
  std::vector<LineStr> ids;
  Token tok;
  while (true) {
    tok = stream.PeekCur();
    if (tok.type != Token::Identifier) {
      err.ReportUnexpectedToken(Token::Identifier, tok);
      return std::nullopt;
    }
    stream.PopCur();
    ids.push_back(*tok.ToLineStr());

    // if there is no dot then this is the end of the identifier access
    if (stream.PeekCur().type != continuer) break;
    stream.PopCur(); // remove the continuer
  }

  LineRange pos = FindRangeOfVector(ids);
  return std::make_unique<IdentifierAccess>(pos, std::move(ids));
}

optional_unique_ptr<TypeStatement> Parser::ParseTypeStatement() {
  Token tok = stream.PeekCur();
  optional_unique_ptr<TypeStatement> ret;

  if (tok.type == Token::Macro) {
    ret = ComposeExpr<TypeStatement>(ParseMacroExpr());
    if (!ret || !ConsumeToken((Token::SemiColon))) return std::nullopt;
  } else if (tok.type == Token::Type) {
    ret = ComposeExpr<TypeStatement>(ParseTypeDecl());
    if (!ret) return std::nullopt;
  } else {
    auto idents = ParseIdentifierAccess(Token::Dot);
    if (!idents) return std::nullopt;
    auto ret = ComposeExpr<TypeStatement>(ParseVarDecl(), std::move(*idents));
    if (!ret) return std::nullopt;

    Token last = stream.LastPopped();
    // if the last token passed was a right brace we don't need a line terminator
    bool requires_terminator = last.type != Token::RightBrace;

    if (requires_terminator || stream.PeekCur().type == Token::SemiColon) {
      // Slightly better error msg
      if (stream.PeekCur().type != Token::SemiColon) {
        err.ReportMissingToken(Token::SemiColon, last.pos);
        return std::nullopt;
      }
      stream.PopCur();
    }
  }
  return ret;
}

optional_unique_ptr<Expr> Parser::ParseFuncExpr(
    std::unique_ptr<TupleValueDecl> tuple_decl) {
  // test for type_expr for functions
  Token tok = stream.PeekCur();
  if (tok.type == Token::FatArrow || tok.type == Token::ReturnType) {
    stream.PopCur();
    tok = stream.PeekCur();
    optional_unique_ptr<TypeExpr> type = std::nullopt;
    if (tok.type == Token::ReturnType) {
      stream.PopCur();
      type = ParseTypeExpr();
      if (!ConsumeToken(Token::FatArrow)) return std::nullopt;
    }

    auto block = ParseBlock();
    if (!block) return std::nullopt;

    LineRange pos = tuple_decl->pos;
    if (block->size() > 0) {
      pos = LineRange(pos, (*block)[block->size()-1]->pos);
    }

    return std::make_unique<Expr>(pos, std::move(tuple_decl),
                                  std::move(type), std::move(*block));
  } else {
    // @IMPROVEMENT: we can give you slightly more tuned msgs
    err.ReportCustomErr("Was expecting a function", tok.pos,
                        ErrStream::SyntaxErr, "=> and/or ->");
    return std::nullopt;
  }
}

std::optional<TupleValueDecl::ArgDecl> Parser::ParseTupleValueDeclSegment() {
  Token tok = stream.PeekCur();

  if (tok.type == Token::Identifier) {
    stream.PopCur();
    auto id = *tok.ToLineStr();
    optional_unique_ptr<TypeExpr> type_expr = std::nullopt;
    tok = stream.PeekCur();
    if (tok.type == Token::Colon) {
      stream.PopCur();
      auto type = ParseTypeExpr();
      if (!type) return std::nullopt;
      type_expr = std::move(*type);
      tok = stream.PeekCur();
    }

    optional_unique_ptr<Expr> val = std::nullopt;
    if (tok.type == Token::Assign) {
      stream.PopCur();
      auto expr = ParseExpr();
      if (!expr) return std::nullopt;
      val = std::move(*expr);
    }

    return TupleValueDecl::ArgDecl(std::move(id), std::move(type_expr),
                                   std::move(val));
  } else {
    err.ReportUnexpectedToken(Token::Identifier, tok);
    return std::nullopt;
  }
}

optional_unique_ptr<TupleValueDecl> Parser::ParseRestTupleValueDeclExpr(
    std::vector<TupleValueDecl::ArgDecl> declarations) {
  // presuming atleast the '(' has been consumed and all has been correctly
  // parsed into declarations
  LineRange start = stream.PeekCur().pos;

  bool extra_comma = false;
  Token prev_tok = Token();
  while (stream.PeekCur().type != Token::RightParen) {
    extra_comma = false;
    auto decl = ParseTupleValueDeclSegment();
    if (!decl) return std::nullopt;
    declarations.push_back(std::move(*decl));

    Token tok = stream.PeekCur();
    if (tok.type != Token::Comma) break;
    prev_tok = tok;
    stream.PopCur(); // pop comma
    extra_comma = true;
  }

  if (extra_comma) err.ReportCustomErr("Extra ','", prev_tok.pos,
                                       ErrStream::SyntaxErr);

  LineRange pos = LineRange(start, stream.PeekCur().pos);

  if (!ConsumeToken(Token::RightParen)) return std::nullopt;

  return std::make_unique<TupleValueDecl>(pos, std::move(declarations));
}

optional_unique_ptr<Expr> Parser::ParseExprOrTupleValueDecl() {
  LineRange start = stream.PeekCur().pos;

  if (!ConsumeToken(Token::LeftParen)) return std::nullopt;

  std::vector<TupleValueDecl::ArgDecl> declarations;
  bool comma = false;
  vector_unique_ptr<Expr> expressions;
  Token tok = stream.PeekCur();
  Token prev_tok;

  while (tok.type != Token::RightParen) {
    comma = false;
    tok = stream.PopCur();
    Token next = stream.PeekCur();

    if (tok.type == Token::Identifier  &&
       (next.type == Token::Colon      ||
        next.type == Token::Assign     ||
        next.type == Token::RightParen ||
        next.type == Token::Comma)) {
      stream.Push(tok);
      auto decl = ParseTupleValueDeclSegment();
      if (!decl) return std::nullopt;
      bool is_tuple_decl = decl->type || decl->expr;
      declarations.push_back(std::move(*decl));

      if (stream.PeekCur().type == Token::Comma) {
        comma = true;
        prev_tok = stream.PopCur();
      }

      if (is_tuple_decl) {
        if (comma && stream.PeekCur().type == Token::RightParen) {
          err.ReportCustomErr("Extra ','", prev_tok.pos,
                              ErrStream::SyntaxErr);
        }

        auto tuple_decl = ParseRestTupleValueDeclExpr(std::move(declarations));
        if (!tuple_decl) return std::nullopt;
        return ParseFuncExpr(std::move(*tuple_decl));
      }
    } else {
      // expression
      stream.Push(tok);
      std::for_each(declarations.cbegin(), declarations.cend(),
        [&expressions, this](auto &decl) {
          expressions.push_back(ConvIdentToExpr(decl.id));
        });

      optional_unique_ptr<Expr> expr;
      while (stream.PeekCur().type != Token::RightParen &&
            (expr = ParseExpr()).has_value()) {
        comma = false;
        expressions.push_back(std::move(*expr));
        if (stream.PeekCur().type != Token::Comma) break;
        prev_tok = stream.PopCur(); // pop comma
        comma = true;
      }

      if (!expr) return std::nullopt;
      break;
    }
    tok = stream.PeekCur();
  }

  if (!ConsumeToken((Token::RightParen))) return std::nullopt;

  tok = stream.PeekCur();
  if (tok.type == Token::FatArrow) {
    // @QUESTION: wattt? is this doing
    if (expressions.size() != 0) {
      err.ReportInvalidToken(tok);
      return std::nullopt;
    }
    if (comma) err.ReportCustomErr("Extra ','", prev_tok.pos,
                                   ErrStream::SyntaxErr);
    LineRange pos = LineRange(start, tok.pos);
    return ParseFuncExpr(std::make_unique<TupleValueDecl>(pos,
      std::move(declarations)));
  }

  // convert all declarations into expressions anyway
  if (expressions.size() == 0) {
    std::for_each(declarations.cbegin(), declarations.cend(),
      [&expressions, this](auto &decl) {
        expressions.push_back(ConvIdentToExpr(decl.id));
      });
  }

  if (expressions.size() == 1 && !comma) {
    // literally just the first expression wrapped in paren
    return ParenthesiseExpr(std::move(expressions[0]));
  } else {
    if (expressions.size() != 1 && comma) {
      err.ReportCustomErr("Extra ','", prev_tok.pos, ErrStream::SyntaxErr);
    }
    // tuple
    LineRange pos = FindRangeOfVector(expressions);
    return std::make_unique<Expr>(pos, std::move(expressions), false);
  }
}

std::vector<LineStr> Parser::ParseIds(Token::Kind continuer) {
  std::vector<LineStr> ids;
  Token tok;

  while ((tok = stream.PeekCur()).type == Token::Identifier) {
    stream.PopCur();
    ids.push_back(*tok.ToLineStr());
    if (stream.PeekCur().type == continuer) {
      stream.PopCur();
    } else {
      break;
    }
  }

  return std::move(ids);
}

optional_unique_ptr<Expr> Parser::ParseVarDeclOrAssignmentExpr() {
  std::vector<LineStr> lhs_str;
  Token tok;
  optional_unique_ptr<Expr> expr = std::nullopt;

  lhs_str = ParseIds(Token::Comma);
  tok = stream.PeekCur();
  if (tok.type == Token::Colon || tok.type == Token::DoubleColon ||
      tok.type == Token::ColonAssign) {
    // is a var decl
    std::vector<VarDecl::Declaration> decls;
    for (auto &id : lhs_str) {
      decls.push_back(VarDecl::Declaration(id, std::nullopt, std::nullopt));
    }
    expr = ComposeExpr<Expr>(ParseRhsVarDecl(std::move(decls)));
  } else if (tok.IsAssignmentOp()) {
    expr = ComposeExpr<Expr>(ParseAssignmentExprVarDeclHint(
      std::move(lhs_str)));
  } else {
    // @NOTE: this kinda destroys a lot of the tokens
    //        when @MULTIPLE_ERRORS comes about we probably
    //        want to do something a tad less destructive.
    // has to be an expr which is invalid here
    err.ReportCustomErr("No assignment occurring after `let`", tok.pos,
                        ErrStream::SyntaxErr);
    return std::nullopt;
  }
  return std::move(expr);
}

optional_unique_ptr<Expr> Parser::ParseExpr() {
  // @CLEANUP: split this up
  Token first = stream.PeekCur();
  optional_unique_ptr<Expr> expr = std::nullopt;

  switch (first.type) {
    case Token::Let: {
      stream.PopCur();
      expr = ParseVarDeclOrAssignmentExpr();
    } break;
    case Token::LeftParen: {
      // tuple/func or simply '(' expr ')'
      Token tok = stream.PopCur();
      Token next = stream.PeekCur();

      // empty tuple or we see something that indicates function
      if (next.type == Token::RightParen) {
        // either func/struct
        stream.Push(tok);
        auto tuple_decl = ParseTupleValueDecl();
        if (!tuple_decl) return std::nullopt;
        expr = ParseFuncExpr(std::move(*tuple_decl));
      } else if (next.type == Token::Identifier) {
        // two trees we care about
        // 1) func where this is a tuple decl
        // 2) tuple/expr where this is well a list of expressions
        stream.Push(tok);
        expr = ParseExprOrTupleValueDecl();
      } else {
        // has to be a tuple
        vector_unique_ptr<Expr> exprs;
        bool comma = false;
        Token last = next;
        while (stream.PeekCur().type != Token::RightParen) {
          comma = false;
          auto expr = ParseExpr();
          if (!expr) return std::nullopt;
          exprs.push_back(std::move(*expr));

          if (stream.PeekCur().type == Token::Comma) {
            last = stream.PopCur();
            comma = true;
          } else if (stream.PeekCur().type != Token::RightParen) {
            err.ReportUnexpectedToken(Token::Comma, stream.PeekCur());
            return std::nullopt;
          }
        }

        if (comma && exprs.size() != 1) {
          err.ReportUnexpectedToken(Token::RightParen, last);
          return std::nullopt;
        }
        LineRange pos = stream.PeekCur().pos;
        if (exprs.size() > 0) pos = FindRangeOfVector(exprs);
        if (!ConsumeToken(Token::RightParen)) return std::nullopt;

        if (!comma && exprs.size() == 1) {
          expr = std::move(exprs[0]);
        } else {
          expr = std::make_unique<Expr>(pos, std::move(exprs), false);
        }
      }
    } break;
    case Token::LeftBracket: {
      // array
      vector_unique_ptr<Expr> vals;
      if (!ParseList(&Parser::ParseExpr, PushBack, vals)) return std::nullopt;
      LineRange pos = FindRangeOfVector(vals);
      expr = std::make_unique<Expr>(pos, std::move(vals), true);
    } break;
    case Token::LeftBrace: {
      auto exprs = ParseBlock();
      if (!exprs) return std::nullopt;
      LineRange pos = first.pos;
      if (exprs->size() > 0) {
        pos = LineRange(pos, (*exprs)[exprs->size()-1]->pos);
      }
      expr = std::make_unique<Expr>(pos, std::move(*exprs));
    } break;
    case Token::If: expr = ComposeExpr<Expr>(ParseIfBlock()); break;
    case Token::For: expr = ComposeExpr<Expr>(ParseForBlock()); break;
    case Token::While: expr = ComposeExpr<Expr>(ParseWhileBlock()); break;
    default: {
      auto logical_expr = ParseLogicalOrExpr();
      if (!logical_expr) return std::nullopt;

      // @FIXME: I don't really want logical exprs here but I'm lazy
      //         and fixing this is going to be a pain.
      //         The nicest fix I can think of right now is just to
      //         check if it folds to additive_expr else we can err.
      if (stream.PeekCur().type == Token::Range) {
        // range based statement
        stream.PopCur();
        bool inclusive_right = false;
        if (stream.PeekCur().type == Token::Assign) {
          inclusive_right = true;
          stream.PopCur();
        }
        auto stop = ParseLogicalOrExpr();
        if (!stop) return std::nullopt;
        LineRange pos = LineRange((*logical_expr)->pos, (*stop)->pos);
        optional_unique_ptr<LogicalOrExpr> step = std::nullopt;
        if (stream.PeekCur().type == Token::Colon) {
          stream.PopCur();
          auto tmp_step = ParseLogicalOrExpr();
          if (!tmp_step) return std::nullopt;
          step = std::move(*tmp_step);
          pos = LineRange(pos, (*step)->pos);
        }
        expr = std::make_unique<Expr>(pos, inclusive_right,
                                      std::move(*logical_expr),
                                      std::move(*stop), std::move(step));
      } else {
        expr = std::make_unique<Expr>((*logical_expr)->pos,
                                      std::move(*logical_expr));
      }
    } break;
  }

  return expr;
}

optional_unique_ptr<TupleValueDecl> Parser::ParseTupleValueDecl() {
  if (!ConsumeToken((Token::LeftParen))) return std::nullopt;
  return ParseRestTupleValueDeclExpr(std::vector<TupleValueDecl::ArgDecl>());
}

optional_unique_ptr<VarDecl> Parser::ParseRhsVarDecl(
    std::vector<VarDecl::Declaration> decls) {
  Token cur = stream.PopCur();
  bool atleast_one;
  if (decls.size() == 0) {
    err.ReportCustomErr("Invalid VarDecl missing lhs", cur.pos,
                        ErrStream::SyntaxErr);
    return std::nullopt;
  }
  LineRange start = decls[0].id.pos;

  bool mut = true;
  if (cur.type == Token::Colon) {
    atleast_one = true;
    // type-expr list
    auto set_type = +[](int index, std::unique_ptr<TypeExpr> type_expr, 
                        std::vector<VarDecl::Declaration> &decls,
                        ErrStream &err, int &count) {
      if (index < decls.size()) {
        decls.at(index).type = std::move(type_expr);
        count++;
      } else {
        err.ReportCustomErr("Too many types for variables", type_expr->pos,
                            ErrStream::SemanticErr);
      }
    };
    int num = 0;
    if (!ParseList(&Parser::ParseTypeExpr, set_type, decls, err, num)) {
      return std::nullopt;
    }
    if (num != 1 && num != decls.size()) {
      // we don't have enough types @TODO: put how many were were expecting
      err.ReportCustomErr("Too few types for variables", stream.PeekCur().pos,
                          ErrStream::SemanticErr);
    }

    cur = stream.PopCur();
  }

  if (cur.type == Token::Assign || cur.type == Token::ColonAssign ||
      cur.type == Token::DoubleColon || cur.type == Token::Colon) {
    if (atleast_one && (cur.type == Token::DoubleColon ||
        cur.type == Token::ColonAssign)) {
      err.ReportCustomErr("Wasn't expecting both ':' or '=' and ':=' or '::'",
                          cur.pos, ErrStream::SyntaxErr);
    }

    atleast_one = true;
    mut = cur.type != Token::DoubleColon && cur.type != Token::Colon;
    // expr list
    auto set_expr = +[](int index, std::unique_ptr<Expr> expr,
                        std::vector<VarDecl::Declaration> &decls,
                        ErrStream &err, int &count) {
      if (index < decls.size()) {
        decls.at(index).expr = std::move(expr);
        count++;
      } else {
        err.ReportCustomErr("Too many exprs for variables", expr->pos,
                            ErrStream::SemanticErr);
      }
    };
    int count;
    if (!ParseList(&Parser::ParseExpr, set_expr, decls, err, count)) {
      return std::nullopt;
    }
    if (count != 1 && count != decls.size()) {
      // we don't have enough types @TODO: put how many were were expecting
      err.ReportCustomErr("Too few exprs for variables", stream.PeekCur().pos,
                          ErrStream::SemanticErr);
    }
  }

  if (!atleast_one) {
    // invalid var_decl
    err.ReportInvalidToken(cur);
    err.ReportCustomErr("Was expecting either ':' or '=' or both.", cur.pos, 
                        ErrStream::SyntaxErr);
    stream.Push(cur);
    return std::nullopt;
  }

  LineRange end = decls[decls.size() - 1].GetPos();
  return std::make_unique<VarDecl>(LineRange(start, end), mut,
                                   std::move(decls));
}


optional_unique_ptr<VarDecl> Parser::ParseVarDecl() {
  // Parse identifier list
  std::vector<VarDecl::Declaration> decls;
  Token cur = stream.PopCur();
  LineRange start = cur.pos;

  while (true) {
    cur = stream.PeekCur();
    if (cur.type != Token::Identifier) {
      int line_end = cur.pos.line_end;
      cur.pos = start;
      cur.pos.line_end = line_end;
      err.ReportInvalidTokenCast(cur, "::, :=, :");
      return std::nullopt;
    }
    stream.PopCur();
    auto id = *cur.ToLineStr();
    decls.push_back(VarDecl::Declaration(id, std::nullopt, std::nullopt));

    // continuer
    if (stream.PeekCur().type != Token::Comma) break;
    stream.PopCur();
  }

  if ((cur = stream.PopCur()).type != Token::Colon &&
      cur.type != Token::DoubleColon &&
      cur.type != Token::ColonAssign) {
    err.ReportInvalidTokenCast(cur, "::, :=, :");
    return std::nullopt;
  }
  return ParseRhsVarDecl(std::move(decls));
}

std::optional<vector_unique_ptr<FuncStatement>> Parser::ParseBlock() {
  Token tok = stream.PeekCur();
  vector_unique_ptr<FuncStatement> exprs;
  optional_unique_ptr<FuncStatement> func_expr;

  if (tok.type == Token::LeftBrace) {
    stream.PopCur();

    if (stream.PeekCur().type != Token::RightBrace) {
      while (stream.PeekCur().type != Token::RightBrace &&
            stream.PeekCur().type != Token::EndOfFile &&
            (func_expr = ParseFuncStatement())) {
        exprs.push_back(std::move(*func_expr));
      }

      if (!ConsumeToken(Token::RightBrace) || !func_expr) {
        return std::nullopt;
      }
    } else {
      stream.PopCur();
    }
  } else {
    func_expr = ParseFuncStatement(false, false);
    if (!func_expr) return std::nullopt;
    // note: we only want to apply the 'block_val' in the case that this is
    //       an expr.  If it is a vardecl/assignment then we don't apply this
    if (std::holds_alternative<std::unique_ptr<Expr>>((*func_expr)->expr) &&
        (*func_expr)->prefix == FuncStatement::NoPrefix) {
      (*func_expr)->prefix = FuncStatement::BlockVal;
    }
    exprs.push_back(std::move(*func_expr));
  }

  return std::move(exprs);
}

std::optional<IfBlock::IfStatement> Parser::ParseIfStatement() {
  if (!ConsumeToken((Token::If))) return std::nullopt;

  auto cond = ParseExpr();
  if (!cond) return std::nullopt;

  auto func_block = ParseBlock();
  if (!func_block) return std::nullopt;
  return IfBlock::IfStatement(std::move(*cond), std::move(*func_block));
}

optional_unique_ptr<IfBlock> Parser::ParseIfBlock() {
  std::vector<IfBlock::IfStatement> statements;
  auto statement = ParseIfStatement();
  if (!statement) return std::nullopt;
  statements.push_back(std::move(*statement));

  while (stream.PeekCur().type == Token::Else) {
    // then either else if or else
    stream.PopCur();
    if (stream.PeekCur().type == Token::If) {
      statement = ParseIfStatement();
      if (!statement) return std::nullopt;
      statements.push_back(std::move(*statement));
    } else {
      // else and then end the function
      auto func_block = ParseBlock();
      if (!func_block) return std::nullopt;
      LineRange end = stream.PeekCur().pos;
      if ((*func_block).size() > 0) end = FindRangeOfVector(*func_block);
      LineRange pos = LineRange(statements[0].cond->pos, end);
      return std::make_unique<IfBlock>(pos, std::move(statements),
                                       std::move(*func_block));
    }
  }
  LineRange end = stream.PeekCur().pos;
  auto &block = statements[statements.size() - 1].block;
  if (block.size() > 0) {
    end = FindRangeOfVector(block);
  }
  LineRange pos = LineRange(statements[0].cond->pos, end);
  return std::make_unique<IfBlock>(pos, std::move(statements), std::nullopt);
}

optional_unique_ptr<WhileBlock> Parser::ParseWhileBlock() {
  if (!ConsumeToken(Token::While)) return std::nullopt;

  auto cond = ParseExpr();
  if (!cond) return std::nullopt;

  auto expr = ParseBlock();
  if (!expr) return std::nullopt;

  LineRange pos = (*cond)->pos;
  if (expr->size() > 0) {
    pos = LineRange(pos, (*expr)[expr->size()-1]->pos);
  }

  return std::make_unique<WhileBlock>(pos, std::move(*cond), std::move(*expr));
}

optional_unique_ptr<ForBlock> Parser::ParseForBlock() {
  LineRange start = stream.PeekCur().pos;
  if (!ConsumeToken(Token::For)) return std::nullopt;

  // to support parenthesis around the `ident_list list 'in' expr_list`
  // we just do it manually, and track if we consumed the '(' so that we
  // can expect the ')'
  bool consumed_lparen = false;
  if (stream.PeekCur().type == Token::LeftParen) {
    consumed_lparen = true;
    if (!ConsumeToken(Token::LeftParen)) return std::nullopt;
  }

  auto ids = ParseIdentifierAccess(Token::Comma);
  if (!ids) return std::nullopt;

  if (!ConsumeToken((Token::In))) return std::nullopt;

  vector_unique_ptr<Expr> exprs;
  if (!ParseList(&Parser::ParseExpr, PushBack, exprs)) return std::nullopt;

  if (consumed_lparen && !ConsumeToken(Token::RightParen)) {
    return std::nullopt;
  }

  auto expr = ParseBlock();
  if (!expr) return std::nullopt;

  LineRange pos = LineRange(start, (*expr)[expr->size()-1]->pos);
  return std::make_unique<ForBlock>(pos, std::move(*ids), std::move(exprs),
                                    std::move(*expr));
}

optional_unique_ptr<FuncCall> Parser::ParseFuncCall() {
  auto func = ParseAtom();
  if (!func) return std::nullopt;

  if (!ConsumeToken(Token::LeftParen)) return std::nullopt;

  vector_unique_ptr<Expr> args;
  if (stream.PeekCur().type != Token::RightParen) {
    // parse arguments
    if (!ParseList(&Parser::ParseExpr, PushBack, args)) return std::nullopt;
  }

  LineRange pos = LineRange((*func)->pos, stream.PeekCur().pos);
  if (!ConsumeToken(Token::RightParen)) return std::nullopt;

  return std::make_unique<FuncCall>(pos, std::move(*func), std::move(args));
}

// no error variant
optional_unique_ptr<Constant> Parser::TryParseConstant() {
  // can be float, int, string, char, or bool
  // luckily for us this should already be parsed except for bool
  Token tok = stream.PopCur();
  switch (tok.type) {
    case Token::Int: {
      return std::make_unique<Constant>(tok.pos, std::get<std::int64_t>(tok.data));
    } break;
    case Token::Flt: {
      return std::make_unique<Constant>(tok.pos, std::get<double>(tok.data));
    } break;
    case Token::Str: {
      return std::make_unique<Constant>(tok.pos, std::get<std::string>(tok.data));
    } break;
    case Token::Char: {
      return std::make_unique<Constant>(tok.pos, std::get<char>(tok.data));
    } break;
    case Token::True: {
      return std::make_unique<Constant>(tok.pos, true);
    } break;
    case Token::False: {
      return std::make_unique<Constant>(tok.pos, false);
    } break;
    default: {
      stream.Push(tok);
      return std::nullopt;
    }
  }
}

optional_unique_ptr<Constant> Parser::ParseConstant() {
  if (auto constant = TryParseConstant()) return std::move(*constant);
  err.ReportCustomErr("Invalid constant", stream.PopCur().pos, ErrStream::SemanticErr);
  return std::nullopt;
}

optional_unique_ptr<MacroExpr> Parser::ParseMacroExpr() {
  if (!ConsumeToken(Token::Macro)) return std::nullopt;
  Token tok = stream.PopCur();
  LineRange pos_start = tok.pos;
  LineRange pos_end = tok.pos;

  auto ids = ParseIdentifierAccess(Token::Dot);
  if (!ids) return std::nullopt;

  if (!ConsumeToken(Token::LeftParen)) return std::nullopt;

  vector_unique_ptr<Expr> exprs;
  while (stream.PeekCur().type != Token::RightParen) {
    auto expr = ParseExpr();
    if (!expr) return std::nullopt;
    exprs.push_back(std::move(*expr));

    tok = stream.PeekCur();
    if (tok.type == Token::Comma) {
      stream.PopCur();
    } else if (tok.type != Token::RightParen) {
      err.ReportUnexpectedToken(Token::RightParen, tok);
      return std::nullopt;
    } else {
      pos_end = tok.pos;
    }
  }

  if (!ConsumeToken(Token::RightParen)) return std::nullopt;

  return std::make_unique<MacroExpr>(LineRange(pos_start, pos_end),
                                     std::move(*ids), std::move(exprs));
}

std::optional<TupleTypeDecl::ArgDecl> Parser::ParseTupleTypeSegment() {
  std::optional<LineStr> id = std::nullopt;
  Token tok = stream.PeekCur();
  if (tok.type == Token::Identifier) {
    id = tok.ToLineStr();
    stream.PopCur();
    if (!ConsumeToken(Token::Colon)) return std::nullopt;
  }

  auto type = ParseTypeExpr();
  if (!type) return std::nullopt;
  return TupleTypeDecl::ArgDecl(id, std::move(*type));
}

optional_unique_ptr<TupleTypeDecl> Parser::ParseTupleType() {
  if (!ConsumeToken(Token::LeftParen)) return std::nullopt;
  std::vector<TupleTypeDecl::ArgDecl> decls;
  LineRange start = stream.PeekCur().pos;

  bool extra_comma = false;
  Token prev_tok = Token();
  while (stream.PeekCur().type != Token::RightParen) {
    extra_comma = false;
    auto decl = ParseTupleTypeSegment();
    if (!decl) return std::nullopt;
    decls.push_back(std::move(*decl));

    Token tok = stream.PeekCur();
    if (tok.type != Token::Comma) break;
    prev_tok = tok;
    stream.PopCur(); // pop comma
    extra_comma = true;
  }

  if (extra_comma) err.ReportCustomErr("Extra ','", prev_tok.pos,
                                       ErrStream::SyntaxErr);
  LineRange pos = LineRange(start, stream.PeekCur().pos);
  if (!ConsumeToken(Token::RightParen)) return std::nullopt;
  return std::make_unique<TupleTypeDecl>(pos, std::move(decls));
}

optional_unique_ptr<TypeExpr> Parser::ParseTypeExpr() {
  Token tok = stream.PeekCur();
  LineRange start = tok.pos;
  optional_unique_ptr<TypeExpr> ret;
  if (tok.type == Token::LeftParen) {
    auto tuple_type = ParseTupleType();
    tok = stream.PeekCur();
    if (tok.type == Token::ReturnType) {
      // function
      stream.PopCur();
      auto ret_type = ParseTypeExpr();
      if (!ret_type) return std::nullopt;
      LineRange range = LineRange((*tuple_type)->pos, (*ret_type)->pos);
      ret = std::make_unique<TypeExpr>(range,
          TypeExpr::FunctionType(std::move(*tuple_type),
                                 std::move(*ret_type)));
    } else {
      // tuple
      ret = ComposeExpr<TypeExpr>(std::move(tuple_type));
    }
  } else if (tok.type == Token::Generic) {
    stream.PopCur();
    tok = stream.PopCur();
    if (tok.type != Token::Identifier) {
      err.ReportUnexpectedToken(Token::Identifier, tok);
      return std::nullopt;
    }
    auto id = *tok.ToLineStr();
    ret = std::make_unique<TypeExpr>(id.pos, std::move(id));
  } else {
    // has to be either just an identifier access or some sort of generic type
    auto id = ParseIdentifierAccess(Token::Dot);
    if (!id) return std::nullopt;

    if (stream.PeekCur().type == Token::LeftBracket) {
      // generic
      vector_unique_ptr<TypeExpr> args;
      LineRange start = stream.PopCur().pos;
      while (stream.PeekCur().type != Token::RightBracket) {
        auto type_expr = ParseTypeExpr();
        if (!type_expr) return std::nullopt;
        args.push_back(std::move(*type_expr));
        if (stream.PeekCur().type != Token::RightBracket) {
          if (!ConsumeToken(Token::Comma)) return std::nullopt;
        }
      }

      LineRange end = stream.PopCur().pos;
      LineRange pos = LineRange(start, end);
      auto generic = TypeExpr::GenericType(std::move(*id), std::move(args));
      ret = std::make_unique<TypeExpr>(pos, std::move(generic));
    } else {
      // just an identifier
      LineRange pos = (*id)->pos;
      ret = std::make_unique<TypeExpr>(pos, std::move(*id));
    }
  }

  if (!ret) return std::nullopt;
  if (stream.PeekCur().type == Token::Variant) {
    stream.PopCur();
    auto next = ParseTypeExpr();
    if (!next) return std::nullopt;

    // check if the next type is a variant if so just move their lhs
    // over to the vector and move us into the lhs off the variant
    // @FIXME: currently we push onto the front of the vector
    //         this is to make the order the way the user expected
    //         but should we also allow for it to push to the back
    //         which is more efficient (or even use a vector that is backwards)
    if (auto variant = std::get_if<TypeExpr::VariantType>(&(*next)->expr)) {
      variant->rhs.insert(variant->rhs.begin(), std::move(variant->lhs));
      variant->lhs = std::move(*ret);
    }
  }
  return ret;
}

optional_unique_ptr<AssignmentExpr> Parser::ParseRhsAssignmentExpr(
    vector_unique_ptr<Expr> lhs) {
  Token cur = stream.PeekCur();
  if (lhs.size() == 0) {
    err.ReportCustomErr("Invalid AssignmentExpr missing lhs", cur.pos,
                        ErrStream::SyntaxErr);
    return std::nullopt;
  }

  auto op = TokenCast<AssignmentOp>(stream.PopCur());
  if (!op) return std::nullopt;

  vector_unique_ptr<Expr> rhs;
  if (!ParseList(&Parser::ParseExpr, PushBack, rhs)) return std::nullopt;

  LineRange pos = LineRange(lhs[0]->pos, rhs[rhs.size() - 1]->pos);
  return std::make_unique<AssignmentExpr>(pos, std::move(lhs), *op,
                                          std::move(rhs));
}

optional_unique_ptr<AssignmentExpr> Parser::ParseAssignmentExprVarDeclHint(
    std::vector<LineStr> ids) {
  // convert the incorrect hint data
  vector_unique_ptr<Expr> lhs;
  for (auto &id : ids) {
    lhs.push_back(ConvIdentToExpr(std::move(id)));
  }

  // we may still have more exprs to read in
  if (!stream.PeekCur().IsAssignmentOp()) {
    if (!ParseList(&Parser::ParseExpr, PushBack, lhs)) return std::nullopt;
  }

  return ParseRhsAssignmentExpr(std::move(lhs));
}

optional_unique_ptr<AssignmentExpr> Parser::ParseAssignmentExpr() {
  vector_unique_ptr<Expr> lhs;
  if (!ParseList(&Parser::ParseExpr, PushBack, lhs)) return std::nullopt;
  return ParseRhsAssignmentExpr(std::move(lhs));
}

optional_unique_ptr<Atom> Parser::ParseAtom() {
  Token peek = stream.PeekCur();
  std::unique_ptr<Atom> atom;

  if (peek.type == Token::LeftParen) {
    // (expr)
    stream.PopCur();
    auto tmp = ParseExpr();
    if (!tmp) return std::nullopt;
    LineRange pos = (*tmp)->pos;
    atom = std::make_unique<Atom>(pos, std::move(*tmp));

    if (!ConsumeToken(Token::RightParen)) return std::nullopt;
  } else if (peek.type == Token::Macro) {
    // @macro
    auto macro = ParseMacroExpr();
    if (!macro) return std::nullopt;
    LineRange pos = (*macro)->pos;
    atom = std::make_unique<Atom>(pos, std::move(*macro));
  } else if (peek.type == Token::Identifier) {
    auto id = *stream.PopCur().ToLineStr();
    LineRange pos = id.pos;
    atom = std::make_unique<Atom>(pos, std::move(id));
  } else if (auto constant = TryParseConstant()) {
    LineRange pos = (*constant)->pos;
    atom = std::make_unique<Atom>(pos, std::move(*constant));
  } else {
    err.ReportCustomErr("Invalid Atom Expr", peek.pos, ErrStream::SyntaxErr);
    return std::nullopt;
  }

  // @TODO: this is real bad, @FIXME:
  while (true) {
    if (stream.PeekCur().type == Token::LeftBracket) {
      // slice or index
      auto tmp = ParseSliceOrIndex(std::move(atom));
      if (!tmp) return std::nullopt;
      atom = std::move(*tmp);
    } else if (stream.PeekCur().type == Token::LeftParen) {
      // func call
      // @FIXME: just copied from func call, should fix
      if (!ConsumeToken(Token::LeftParen)) return std::nullopt;
      vector_unique_ptr<Expr> args;
      if (stream.PeekCur().type != Token::RightParen) {
        // parse arguments
        if (!ParseList(&Parser::ParseExpr, PushBack, args)) return std::nullopt;
      }
      LineRange pos = LineRange(atom->pos, stream.PeekCur().pos);
      if (!ConsumeToken(Token::RightParen)) return std::nullopt;
      auto func = std::make_unique<FuncCall>(pos, std::move(atom),
                                             std::move(args));
      atom = std::make_unique<Atom>(pos, std::move(func));
    } else {
      break;
    }
  }

  if (stream.PeekCur().type == Token::FoldLeft) {
    stream.PopCur();
    auto rhs = ParseAdditiveExpr();
    if (!rhs) return std::nullopt;

    LineRange pos = LineRange(atom->pos, (*rhs)->pos);
    atom = std::make_unique<Atom>(pos, std::move(atom), false,
                                  std::move(*rhs));
  }

  return atom;
}

optional_unique_ptr<Atom> Parser::ParseSliceOrIndex(std::unique_ptr<Atom> lhs) {
  if (!ConsumeToken(Token::LeftBracket)) return std::nullopt;
  optional_unique_ptr<Expr> start;
  optional_unique_ptr<Expr> stop;
  optional_unique_ptr<Expr> step;

  // this is a little arcane, with how similar the ifs are are
  // maybe we want some way to do it nicer
  if (stream.PeekCur().type != Token::Colon) {
    // not skipping start
    start = ParseExpr();
  }

  if (stream.PeekCur().type == Token::Colon) {
    // stop and step possibly
    stream.PopCur();
    if (stream.PeekCur().type != Token::Colon) {
      // stop
      stop = ParseExpr();
    }

    if (stream.PeekCur().type == Token::Colon) {
      // step possibly
      stream.PopCur();
      if (stream.PeekCur().type != Token::Colon) {
        // step
        step = ParseExpr();
      }
    }
  }

  LineRange pos = LineRange(lhs->pos, stream.PeekCur().pos);
  if (!ConsumeToken(Token::RightBracket)) return std::nullopt;

  if (!stop && !step && start) {
    // index
    return std::make_unique<Atom>(pos, std::move(lhs), std::move(*start));
  } else {
    return std::make_unique<Atom>(pos, std::move(lhs), std::move(start),
                                  std::move(stop), std::move(step));
  }
}

optional_unique_ptr<UnaryExpr> Parser::ParseUnaryExpr() {
  std::vector<PrefixOp> ops;
  LineRange start = stream.PeekCur().pos;
  while (auto op = PrefixOp::FromToken(stream.PeekCur())) {
    stream.PopCur();
    ops.push_back(*op);
  }

  auto rhs = ParsePowerExpr();
  if (!rhs) return std::nullopt;

  LineRange pos = LineRange(start, (*rhs)->pos);
  return std::make_unique<UnaryExpr>(pos, std::move(*rhs), std::move(ops));
}

optional_unique_ptr<PowerExpr> Parser::ParsePowerExpr() {
  vector_unique_ptr<Atom> exprs;
  if (!ParseList<Token::Power>(&Parser::ParseAtom, PushBack, exprs))
    return std::nullopt;

  LineRange pos = FindRangeOfVector(exprs);
  return std::make_unique<PowerExpr>(pos, std::move(exprs));
}

optional_unique_ptr<MultiplicativeExpr> Parser::ParseMultiplicativeExpr() {
  return ParseOpList<MultiplicativeOp,
                     MultiplicativeExpr>(&Parser::ParseUnaryExpr);
}

optional_unique_ptr<AdditiveExpr> Parser::ParseAdditiveExpr() {
  optional_unique_ptr<AdditiveExpr> additive = ParseOpList<AdditiveOp,
      AdditiveExpr>(&Parser::ParseMultiplicativeExpr);
  // we have to apply it like this incase you have a |> b() |> c()
  // which is c(b(a))
  while (additive && stream.PeekCur().type == Token::FoldRight) {
    stream.PopCur();
    auto rhs = ParseAtom();
    if (!rhs) return std::nullopt;

    LineRange pos = LineRange((*additive)->pos, (*rhs)->pos);
    additive = ExprToFold(std::move(*additive), true, pos, std::move(*rhs));
  }
  return additive;
}

optional_unique_ptr<ComparisonExpr> Parser::ParseComparisonExpr() {
  return ParseOpList<ComparisonOp, ComparisonExpr>(&Parser::ParseAdditiveExpr);
}

optional_unique_ptr<LogicalAndExpr> Parser::ParseLogicalAndExpr() {
  vector_unique_ptr<ComparisonExpr> exprs;
  if (!ParseList<Token::And>(&Parser::ParseComparisonExpr, PushBack, exprs))
    return std::nullopt;

  LineRange pos = FindRangeOfVector(exprs);
  return std::make_unique<LogicalAndExpr>(pos, std::move(exprs));
}

optional_unique_ptr<LogicalOrExpr> Parser::ParseLogicalOrExpr() {
  vector_unique_ptr<LogicalAndExpr> exprs;
  if (!ParseList<Token::Or>(&Parser::ParseLogicalAndExpr, PushBack, exprs))
    return std::nullopt;

  LineRange pos = FindRangeOfVector(exprs);
  return std::make_unique<LogicalOrExpr>(pos, std::move(exprs));
}

}