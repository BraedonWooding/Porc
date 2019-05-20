#include "parser.hpp"

#include <rang.hpp>

#include "helper.hpp"

namespace porc::internals {

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
LineRange GetLineRangeForVec(std::vector<std::unique_ptr<T>> &vec, int index) {
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
    return std::make_unique<T>(expr->pos, std::move(expr), args...);
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
  std::vector<std::unique_ptr<FuncBlock>> exprs;
  std::vector<std::unique_ptr<TypeDecl>> types;

  while ((auto tok = stream.PeekCur()).type != Token::EndOfFile) {
    if (tok.type == Token::Type) {
      if (auto type = ParseTypeDecl()) {
        types.push_back(std::move(*type));
      } else {
        // @TODO: recover
        return std::nullopt;
      }
    } else {
      if (auto expr = ParseFuncBlock(true)) {
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

FuncBlock::PrefixKind Parser::ParseFuncBlockPrefix() {
  FuncBlock::PrefixKind ret = FuncBlock::NoPrefix;
  if (stream.PeekCur().type == Token::Yield) {
    stream.PopCur();
    ret |= FuncBlock::Yield;
  }

  switch (stream.PeekCur().type) {
    case Token::Return: {
      stream.PopCur();
      ret |= FuncBlock::Return;
    } break;
    case Token::Continue: {
      stream.PopCur();
      ret |= FuncBlock::Continue;
    } break;
    case Token::Break: {
      stream.PopCur();
      ret |= FuncBlock::Break;
    } break;
    case Token::Assign: {
      stream.PopCur();
      ret |= FuncBlock::BlockVal;
    } break;
  }

  return ret;
}

optional_unique_ptr<FuncBlock> Parser::ParseFuncBlock(bool file_scope) {
  Token tok = stream.PeekCur();
  LineRange start = tok.pos;
  optional_unique_ptr<FuncBlock> expr;

  bool is_block_decl = false;
  FuncBlock::PrefixKind ret = ParseFuncBlockPrefix();
  if (file_scope && ret != Prefix::NoPrefix) {
    err.ReportCustomErr("Can't yield, return, continue, evaluate, or break"
                        " from file scope", tok.pos, ErrStream::SemanticErr);
    return std::nullopt;
  }

  std::vector<LineStr> lhs_str;
  while (true) {
    tok = stream.PeekCur();
    if (tok.type == Token::Identifier) {
      stream.PopCur();
      lhs_str.push_back(*tok.ToLineStr());
    } else if (tok.type == Token::Colon || tok.type == Token::DoubleColon ||
               tok.type == Token::ColonAssign) {
      // is a var decl
      if (ret != Prefix::NoPrefix) {
        err.ReportCustomErr("Can't yield, return, continue, evaluate, or break "
                            "a var decl.", start, ErrStream::SemanticErr);
        return std::nullopt;
      }
      // turn the line strings into declarations
      std::vector<VarDecl::Declaration> decls;
      for (auto &id : lhs_str) {
        decl.push_back(VarDecl::Declaration(id, std::nullopt, std::nullopt));
      }
      return ParseRhsVarDecl(std::move(decls));
    } else if (tok.IsAssignmentOp()) {
      
    } else {
      // either assignment or expr
      
    }
  }

  std::vector<std::unique_ptr<Expr>> lhs;
  if (!ParseList(&Parser::ParseExpr, PushBack, lhs)) return std::nullopt;

  // it could be that it was just an expression so check if next is operator
  // or that we had more than one
  if (lhs.size() > 1 || stream.PeekCur().IsAssignmentOp()) {
    auto assign_op = TokenCast<AssignmentOp>(stream.PopCur());
    if (!assign_op) return std::nullopt;

    std::vector<std::unique_ptr<Expr>> rhs;
    if (!ParseList(&Parser::ParseExpr, PushBack, rhs)) return std::nullopt;
    LineRange pos = LineRange(start, rhs[rhs.size() - 1]->pos);
    auto assign_expr = std::make_unique<AssignmentExpr>(pos, std::move(lhs),
                                                        *assign_op, 
                                                        std::move(rhs));
    expr = std::make_unique<FuncBlock>(pos, std::move(assign_expr));
  } else {
    // else just an expression
    LineRange pos = lhs[0]->pos;
    expr = std::make_unique<FuncBlock>(pos, std::move(lhs[0]));
  }

  if (!expr) return std::nullopt;

  bool requires_comma = file_scope;
  if (std::holds_alternative<std::unique_ptr<VarDecl>>((*expr)->expr)) {
    requires_comma = !is_block_decl;
  }

  tok = stream.PeekCur();
  if (requires_comma || tok.type == Token::SemiColon) {
    if (tok.type != Token::SemiColon) {
      err.ReportMissingToken(Token::SemiColon, (*expr)->pos.End(1));
      return std::nullopt;
    }
    stream.PopCur();
  } else if (!(*expr)->IsBlockExpr()) {
    ret = true;
  }

  (*expr)->ret = ret;
  return expr;
}

optional_unique_ptr<TypeDecl> ParseTypeDecl() {
  LineRange start = stream.PeekCur().pos;
  if (!ConsumeToken(Token::Type)) return std::nullopt;

  Token tok = stream.PopCur();
  if (tok.type != Token::Identifier) {
    err.ReportUnexpectedToken(Token::Identifier, tok);
    return std::nullopt;
  }
  auto id = *tok.ToLineStr();
  std::optional<std::unique_ptr<TypeExpr>> type = std::nullopt;
  std::vector<std::unique_ptr<StructBlock>> block;

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
    std::optional<std::unique_ptr<StructBlock>> expr;
    while ((auto tok = stream.PeekCur()).type != Token::RightBrace &&
          tok.type != Token::EndOfFile && expr = ParseStructBlock()) {
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
//   auto tuple_decl = ParseTupleDecl();
//   if (!tuple_decl) return std::nullopt;

//   std::optional<std::unique_ptr<TypeExpr>> ret_type = std::nullopt;
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

std::unique_ptr<Expr> Parser::ExprToFold(std::unique_ptr<Expr> expr,
                                         bool folding, LineRange pos,
                                         std::unique_ptr<Atom> func) {
  return std::make_unique<Expr>(pos,
         std::make_unique<LogicalOrExpr>(pos,
         std::make_unique<LogicalAndExpr>(pos,
         std::make_unique<ComparisonExpr>(pos,
         std::make_unique<AdditiveExpr>(pos,
         std::make_unique<MultiplicativeExpr>(pos,
         std::make_unique<UnaryExpr>(pos,
         std::make_unique<PowerExpr>(pos,
         std::make_unique<Atom>(pos, std::move(func), folding,
                                std::move(expr))))))))));
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

// optional_unique_ptr<VarDecl> Parser::ParseStructDecl() {
//   if (!ConsumeToken((Token::Struct))) return std::nullopt;

//   Token tok = stream.PopCur();
//   if (tok.type != Token::Identifier) {
//     err.ReportUnexpectedToken(Token::Identifier, tok);
//     return std::nullopt;
//   }

//   auto id = *tok.ToLineStr();
//   auto tuple_decl = ParseTupleDecl();
//   if (!tuple_decl) return std::nullopt;

//   if (!ConsumeToken((Token::LeftBrace))) return std::nullopt;

//   optional_unique_ptr<StructBlock> expr;
//   std::vector<std::unique_ptr<StructBlock>> exprs;

//   while (stream.PeekCur().type != Token::RightBrace &&
//         (expr = ParseStructBlock()).has_value()) {
//     exprs.push_back(std::move(*expr));
//   }

//   LineRange end = stream.PeekCur().pos;
//   if (exprs.size() > 0) end = FindRangeOfVector(exprs);

//   if (!ConsumeToken((Token::RightBrace))) return std::nullopt;

//   LineRange pos = LineRange((*tuple_decl)->pos, end);
//   auto tmp = std::make_unique<Expr>(pos, std::move(*tuple_decl),
//                                     std::move(exprs));
//   std::vector<VarDecl::Declaration> decl;
//   decl.push_back(VarDecl::Declaration(id, std::nullopt, std::move(tmp)));
//   return std::make_unique<VarDecl>(pos, false, std::move(decl));
// }

optional_unique_ptr<StructBlock> Parser::ParseStructBlock() {
  Token tok = stream.PeekCur();
  optional_unique_ptr<StructBlock> ret;

  if (tok.type == Token::Macro) {
    ret = ComposeExpr<StructBlock>(ParseMacroExpr());
    if (!ret || !ConsumeToken((Token::SemiColon))) return std::nullopt;
  } else if (tok.type == Token::Type) {
    ret = ComposeExpr<StructBlock>(ParseTypeDecl());
    if (!ret) return std::nullopt;
  } else {
    std::vector<LineStr> idents;
    while (tok.type == Token::Identifier) {
      tok = stream.PopCur();
      if (stream.PeekCur().type == Token::Dot) {
        // continue
        idents.push_back(*tok.ToLineStr());
      } else {
        // we need to stop here
        stream.Push(tok);
        break;
      }
    }
    auto ret = ComposeExpr<StructBlock>(ParseVarDecl(), std::move(idents));
    // @TODO: I'm too lazy rn to properly implement semicolons for functions
    // @PLS:  It shouldn't be insane just spend a day on getting it right
    if (!ret || !ConsumeToken(Token::SemiColon)) return std::nullopt;
  }
}

optional_unique_ptr<Expr> Parser::ParseExprFuncOrStruct(
    std::unique_ptr<TupleDecl> tuple_decl) {
  // test for type_expr for functions
  Token tok = stream.PeekCur();
  if (tok.type == Token::DoubleColon) {
    stream.PopCur();
    if (!ConsumeToken((Token::LeftBrace))) return std::nullopt;
    std::vector<std::unique_ptr<StructBlock>> expressions;
    optional_unique_ptr<StructBlock> expr;
    while (stream.PeekCur().type != Token::RightBrace &&
          expr = ParseStructBlock()) {
      expressions.push_back(std::move(*expr));
    }
    if (!expr) return std::nullopt;

    LineRange range = stream.PeekCur().pos;
    if (expressions.size() > 0) range = FindRangeOfVector(expressions);

    if (!ConsumeToken((Token::RightBrace))) return std::nullopt;
    return std::make_unique<Expr>(range, std::move(tuple_decl),
                                  std::move(expressions));
  } else if (tok.type == Token::FatArrow) {
    stream.PopCur();
    auto block = ParseBlock();
    if (!block) return std::nullopt;

    LineRange pos = LineRange(tuple_decl->pos, (*block)[block->size()-1]->pos);
    return std::make_unique<Expr>(pos, std::move(tuple_decl),
                                  std::nullopt,  std::move(*block));
  } else {
    // has to be function just with a type
    auto ret = ParseTypeExpr();
    if (!ret) return std::nullopt;
    if (!ConsumeToken((Token::FatArrow))) return std::nullopt;
    auto block = ParseBlock();
    if (!block) return std::nullopt;
    LineRange pos = LineRange(tuple_decl->pos, (*block)[block->size()-1]->pos);
    return std::make_unique<Expr>(pos, std::move(tuple_decl),
                                  std::move(*ret), std::move(*block));
  }
}

std::optional<TupleDecl::ArgDecl> Parser::ParseTupleDeclSegment() {
  Token tok = stream.PeekCur();
  bool is_mut = false;
  bool is_generic = false;
  if (tok.type == Token::Mut) {
    stream.PopCur();
    is_mut = true;
    tok = stream.PeekCur();
  }

  if (tok.type == Token::Generic) {
    stream.PopCur();
    is_generic = true;
    tok = stream.PeekCur();
  }

  if (tok.type == Token::Identifier) {
    stream.PopCur();
    auto id = *tok.ToLineStr();
    std::optional<std::unique_ptr<TypeExpr>> type_expr = std::nullopt;
    tok = stream.PeekCur();
    if (tok.type == Token::Colon) {
      stream.PopCur();
      auto type = ParseTypeExpr();
      if (!type) return std::nullopt;
      type_expr = std::move(*type);
      tok = stream.PeekCur();
    }

    std::optional<std::unique_ptr<Expr>> val = std::nullopt;
    if (tok.type == Token::Assign) {
      stream.PopCur();
      auto expr = ParseExpr();
      if (!expr) return std::nullopt;
      val = std::move(*expr);
    }

    return TupleDecl::ArgDecl(is_mut, is_generic, std::move(id),
                              std::move(type_expr), std::move(val));
  } else {
    err.ReportUnexpectedToken(Token::Identifier, tok);
    return std::nullopt;
  }
}

optional_unique_ptr<TupleDecl> Parser::ParseRestTupleDeclExpr(
    std::vector<TupleDecl::ArgDecl> declarations) {
  // presuming atleast the '(' has been consumed and all has been correctly
  // parsed into declarations
  LineRange start = stream.PeekCur().pos;

  bool extra_comma = false;
  Token prev_tok = Token();
  while (stream.PeekCur().type != Token::RightParen) {
    extra_comma = false;
    auto decl = ParseTupleDeclSegment();
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

  return std::make_unique<TupleDecl>(pos, std::move(declarations));
}

optional_unique_ptr<Expr> Parser::ParseExprOrTupleDecl() {
  LineRange start = stream.PeekCur().pos;

  if (!ConsumeToken(Token::LeftParen)) return std::nullopt;

  std::vector<TupleDecl::ArgDecl> declarations;
  bool comma = false;
  std::vector<std::unique_ptr<Expr>> expressions;
  Token tok = stream.PeekCur();
  Token prev_tok;


  while (tok.type != Token::RightParen) {
    comma = false;
    tok = stream.PopCur();

    // @TODO: I don't know if this is right (just changed from const)
    //        I'll come back to it later.
    if (tok.type == Token::Mut) {
      stream.Push(tok);
      auto tuple_decl = ParseRestTupleDeclExpr(std::move(declarations));
      if (!tuple_decl) return std::nullopt;
      return ParseExprFuncOrStruct(std::move(*tuple_decl));
    }
    Token next = stream.PeekCur();

    if (tok.type == Token::Identifier  &&
       (next.type == Token::Colon      ||
        next.type == Token::Assign     ||
        next.type == Token::RightParen ||
        next.type == Token::Comma)) {
      stream.Push(tok);
      auto decl = ParseTupleDeclSegment();
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

        auto tuple_decl = ParseRestTupleDeclExpr(std::move(declarations));
        if (!tuple_decl) return std::nullopt;
        return ParseExprFuncOrStruct(std::move(*tuple_decl));
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
  if (tok.type == Token::DoubleColon ||
      tok.type == Token::FatArrow) {
    // @QUESTION: wattt? is this doing
    if (expressions.size() != 0) {
      err.ReportInvalidToken(tok);
      return std::nullopt;
    }
    if (comma) err.ReportCustomErr("Extra ','", prev_tok.pos,
                                   ErrStream::SyntaxErr);
    LineRange pos = LineRange(start, tok.pos);
    return ParseExprFuncOrStruct(std::make_unique<TupleDecl>(pos,
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
    if (expressions.size() != 1 && comma)
      err.ReportCustomErr("Extra ','", prev_tok.pos,
                          ErrStream::SyntaxErr);
    // tuple
    LineRange pos = FindRangeOfVector(expressions);
    return std::make_unique<Expr>(pos, std::move(expressions), false);
  }
}

optional_unique_ptr<Expr> Parser::ParseExpr() {
  // @CLEANUP: split this up
  Token first = stream.PeekCur();
  optional_unique_ptr<Expr> expr = std::nullopt;

  switch (first.type) {
    case Token::Mut: case Token::Const: {
      expr = ComposeExpr<Expr>(ParseVarDecl());
    } break;
    case Token::LeftParen: {
      // tuple/func/struct or simply '(' expr ')'
      Token tok = stream.PopCur();
      Token next = stream.PeekCur();

      // empty tuple or we see something that indicates function
      if (next.type == Token::Mut || next.type == Token::RightParen) {
        // either func/struct
        stream.Push(tok);
        auto tuple_decl = ParseTupleDecl();
        if (!tuple_decl) return std::nullopt;
        expr = ParseExprFuncOrStruct(std::move(*tuple_decl));
      } else if (next.type == Token::Identifier) {
        // two trees we care about
        // 1) func/struct where this is a tuple decl
        // 2) tuple/expr where this is well a list of expressions
        stream.Push(tok);
        expr = ParseExprOrTupleDecl();
      } else {
        // has to be a tuple
        std::vector<std::unique_ptr<Expr>> exprs;
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
      // array/map
      auto first_val = ParseExpr();
      if (!first_val) return std::nullopt;
      if (stream.PeekCur().type == Token::Colon) {
        // we have a map
        std::vector<std::unique_ptr<Expr>> keys;
        std::vector<std::unique_ptr<Expr>> values;
        keys.push_back(std::move(*first_val));

        first_val = ParseExpr();
        if (!first_val) return std::nullopt;
        values.push_back(std::move(*first_val));

        if (!ParseListConjugate(&Parser::ParseExpr, PushBackMap, keys, values))
          return std::nullopt;

        LineRange pos = LineRange(keys[0]->pos, values[values.size() - 1]->pos);
        expr = std::make_unique<Expr>(pos, std::move(keys), std::move(values));
      } else {
        std::vector<std::unique_ptr<Expr>> vals;
        vals.push_back(std::move(*first_val));
        if (!ParseList(&Parser::ParseExpr, PushBack, vals)) return std::nullopt;
        LineRange pos = FindRangeOfVector(vals);
        expr = std::make_unique<Expr>(pos, std::move(vals), true);
      }
    } break;
    case Token::LeftBrace: {
      auto exprs = ParseBlock();
      if (!exprs) return std::nullopt;
      LineRange pos = FindRangeOfVector(*exprs);
      expr = std::make_unique<Expr>(pos, std::move(*exprs));
    } break;
    case Token::If: expr = ComposeExpr<Expr>(ParseIfBlock()); break;
    case Token::For: expr = ComposeExpr<Expr>(ParseForBlock()); break;
    case Token::While: expr = ComposeExpr<Expr>(ParseWhileBlock()); break;
    default: {
      auto logical_expr = ParseLogicalOrExpr();
      if (!logical_expr) return std::nullopt;
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
        std::optional<std::unique_ptr<LogicalOrExpr>> step = std::nullopt;
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

  if (expr && stream.PeekCur().type == Token::FoldRight) {
    stream.PopCur();
    auto rhs = ParseAtom();
    if (!rhs) return std::nullopt;

    LineRange pos = LineRange((*expr)->pos, (*rhs)->pos);
    expr = ExprToFold(std::move(*expr), true, pos, std::move(*rhs));
  }

  return expr;
}

optional_unique_ptr<TupleDecl> Parser::ParseTupleDecl() {
  if (!ConsumeToken((Token::LeftParen))) return std::nullopt;
  return ParseRestTupleDeclExpr(std::vector<TupleDecl::ArgDecl>());
}

optional_unique_ptr<VarDecl> Parser::ParseRhsVarDecl(
    std::vector<VarDecl::Declaration> decls) {
  bool mut = true;
  if (cur.type == Token::Colon) {
    atleast_one = true;
    // type-expr list
    auto set_type = +[](int index, std::unique_ptr<TypeExpr> type_expr, 
                             std::vector<VarDecl::Declaration> &decls) {
      decls.at(index).type = std::move(type_expr);
    };
    if (!ParseList(&Parser::ParseTypeExpr, set_type, decls)) {
      return std::nullopt;
    }
    if (stream.PeekCur().type == Token::Assign) cur = stream.PopCur();
  }

  if (cur.type == Token::Assign || cur.type == Token::ColonAssign ||
      cur.type == Token::DoubleColon || cur.type == Token::Colon) {
    atleast_one = true;
    mut = cur.type != Token::DoubleColon && cur.type != Token::Colon;
    // expr list
    auto set_expr = +[](int index, std::unique_ptr<Expr> expr, 
                             std::vector<VarDecl::Declaration> &decls) {
      decls.at(index).expr = std::move(expr);
    };
    if (!ParseList(&Parser::ParseExpr, set_expr, decls)) return std::nullopt;
  }

  if (!atleast_one) {
    // invalid var_decl
    err.ReportInvalidToken(cur);
    err.ReportCustomErr("Was expecting either ':' or '=' or both.", cur.pos, 
                        ErrStream::SyntaxErr);
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

std::optional<std::vector<std::unique_ptr<FuncBlock>>> Parser::ParseBlock() {
  if (!ConsumeToken(Token::LeftBrace)) return std::nullopt;
  std::vector<std::unique_ptr<FuncBlock>> exprs;
  optional_unique_ptr<FuncBlock> func_expr;

  while (stream.PeekCur().type != Token::RightBrace &&
        stream.PeekCur().type != Token::EndOfFile &&
        func_expr = ParseFuncBlock()) {
    exprs.push_back(std::move(*func_expr));
  }

  if (!ConsumeToken(Token::RightBrace) || !func_expr) return std::nullopt;
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

  LineRange pos = LineRange((*cond)->pos, (*expr)[expr->size()-1]->pos);
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

  std::vector<std::unique_ptr<Expr>> exprs;
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

  std::vector<std::unique_ptr<Expr>> args;
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

  std::vector<std::unique_ptr<Expr>> exprs;
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

optional_unique_ptr<TypeExpr> Parser::ParseTypeExpr() {
  Token tok = stream.PeekCur();
  LineRange start = tok.pos;
  optional_unique_ptr<TypeExpr> ret;
  if (tok.type == Token::LeftParen) {
    // tuple
    ret = ComposeExpr<TypeExpr>(ParseTupleDecl());
  } else if (tok.type == Token::Func) {
    // fn pointer/signature
    stream.PopCur();
    tok = stream.PeekCur();
    std::optional<LineStr> id;
    if (tok.type == Token::Identifier) {
      id = tok.ToLineStr();
      stream.PopCur();
    }
    auto decl = ParseTupleDecl();
    if (!decl) return std::nullopt;

    optional_unique_ptr<TypeExpr> ret_type = std::nullopt;
    if (stream.PeekCur().type == Token::ReturnType) {
      auto ret = ParseTypeExpr();
      if (!ret) return std::nullopt;
      ret_type = std::move(*ret);
    }
    LineRange end = ret_type ? (*ret_type)->pos : (*decl)->pos;
    LineRange pos = LineRange(start, end);
    auto func_type = TypeExpr::FunctionType(std::move(id), std::move(*decl),
                                            std::move(ret_type));
    ret = std::make_unique<TypeExpr>(pos, std::move(func_type));
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
      TypeExpr::GenericType::GenericArgs args;
      LineRange start = stream.PopCur().pos;
      while (stream.PeekCur().type != Token::RightBracket) {
        // @TODO: support more things??
        //        I don't know how far we want to go...?
        //        currently I'm thinking just variables as well
        if (auto constant = TryParseConstant()) {
          args.push_back(std::move(*constant));
        } else {
          auto type_expr = ParseTypeExpr();
          if (!type_expr) return std::nullopt;
          args.push_back(std::move(*type_expr));
        }
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
    std::vector<std::unique_ptr<Expr>> lhs) {
  auto op = TokenCast<AssignmentOp>(stream.PopCur());
  if (!op) return std::nullopt;

  std::vector<std::unique_ptr<Expr>> rhs;
  if (!ParseList(&Parser::ParseExpr, PushBack, rhs)) return std::nullopt;

  LineRange pos = LineRange(lhs[0]->pos, rhs[rhs.size() - 1]->pos);
  return std::make_unique<AssignmentExpr>(pos, std::move(lhs), *op,
                                          std::move(rhs));
}

optional_unique_ptr<AssignmentExpr> Parser::ParseAssignmentExprVarDeclHint(
    std::vector<LineStr> ids) {
  // convert the incorrect hint data
  std::vector<std::unique_ptr<Expr>> lhs;
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
  std::vector<std::unique_ptr<Expr>> lhs;
  if (!ParseList(&Parser::ParseExpr, PushBack, lhs)) return std::nullopt;
  return ParseRhsAssignmentExpr(std::move(lhs));
}

// std::optional<Expr::MapExpr> Parser::ParseMapExpr() {
//   if (!ConsumeToken(Token::LeftBracket)) return std::nullopt;

//   std::vector<std::unique_ptr<Expr>> keys;
//   std::vector<std::unique_ptr<Expr>> values;
//   if (!ParseListConjugate(&Parser::ParseExpr, PushBackMap, keys, values))
//     return std::nullopt;

//   if (!ConsumeToken(Token::RightBracket)) return std::nullopt;

//   return Expr::MapExpr(std::move(keys), std::move(values));
// }

// std::optional<Expr::CollectionExpr> Parser::ParseArrayExpr() {
//   if (!ConsumeToken(Token::LeftBracket)) return std::nullopt;

//   std::vector<std::unique_ptr<Expr>> values;
//   if (!ParseList(&Parser::ParseExpr, PushBack, values)) return std::nullopt;

//   if (!ConsumeToken(Token::RightBracket)) return std::nullopt;

//   return Expr::CollectionExpr(std::move(values), true);
// }

// std::optional<Expr::CollectionExpr> Parser::ParseTupleExpr() {
//   if (!ConsumeToken(Token::LeftParen)) return std::nullopt;

//   std::vector<std::unique_ptr<Expr>> values;
//   if (!ParseList(&Parser::ParseExpr, PushBack, values)) return std::nullopt;

//   if (!ConsumeToken(Token::RightParen)) return std::nullopt;

//   return Expr::CollectionExpr(std::move(values), false);
// }

optional_unique_ptr<Atom> Parser::ParseAtom() {
  Token peek = stream.PeekCur();
  auto prefix_op = PrefixOp::FromToken(peek);
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
      std::vector<std::unique_ptr<Expr>> args;
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
    auto rhs = ParseExpr();
    if (!rhs) return std::nullopt;

    LineRange pos = LineRange(atom->pos, (*rhs)->pos);
    atom = std::make_unique<Atom>(pos, std::move(atom), false,
                                  std::move(*rhs));
  }

  return atom;
}

optional_unique_ptr<Atom> Parser::ParseSliceOrIndex(std::unique_ptr<Atom> lhs) {
  if (!ConsumeToken(Token::LeftBracket)) return std::nullopt;
  std::optional<std::unique_ptr<Expr>> start;
  std::optional<std::unique_ptr<Expr>> stop;
  std::optional<std::unique_ptr<Expr>> step;

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
  std::vector<std::unique_ptr<Atom>> exprs;
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
  return ParseOpList<AdditiveOp,
                     AdditiveExpr>(&Parser::ParseMultiplicativeExpr);
}

optional_unique_ptr<ComparisonExpr> Parser::ParseComparisonExpr() {
  return ParseOpList<ComparisonOp, ComparisonExpr>(&Parser::ParseAdditiveExpr);
}

optional_unique_ptr<LogicalAndExpr> Parser::ParseLogicalAndExpr() {
  std::vector<std::unique_ptr<ComparisonExpr>> exprs;
  if (!ParseList<Token::And>(&Parser::ParseComparisonExpr, PushBack, exprs))
    return std::nullopt;

  LineRange pos = FindRangeOfVector(exprs);
  return std::make_unique<LogicalAndExpr>(pos, std::move(exprs));
}

optional_unique_ptr<LogicalOrExpr> Parser::ParseLogicalOrExpr() {
  std::vector<std::unique_ptr<LogicalAndExpr>> exprs;
  if (!ParseList<Token::Or>(&Parser::ParseLogicalAndExpr, PushBack, exprs))
    return std::nullopt;

  LineRange pos = FindRangeOfVector(exprs);
  return std::make_unique<LogicalOrExpr>(pos, std::move(exprs));
}

}