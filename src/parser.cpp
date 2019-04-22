#include "parser.hpp"

#include <rang.hpp>

#include "helper.hpp"

namespace porc::internals {

/*
  As to make sure that we give much nicer errors we split our errors up like this;
  - tokenizer errors (for example 3e + 2)
  - syntatical errors (for example forgetting semicolons)
  - semantic/lexical errors (invalid types or whatever)

  It will stop at building the AST if there are any tokenizer/syntatical errors
  But will still try to build the AST it gets around this by basically skipping
  any problematic expressions, since we don't care about producing code for this
  and just to report the errors we don't have to care about producing an
  accurate AST (it is going to be wrong regardless).
*/

bool Parser::ConsumeToken(Token::Kind wanted) {
  Token tok = stream.PeekCur();
  // check if next token is what we want
  if (tok.type == Token::EndOfFile) {
    err.ReportExpectedToken(wanted, tok.pos);
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
std::optional<To> Parser::TryTokenCast(Token tok) {
  std::optional<To> cast = To::FromToken(tok);
  if (!cast) {
    err.ReportInvalidTokenCast(tok, To::AllMsg());
  }
  return cast;
}

optional_unique_ptr<FileDecl> Parser::ParseFileDecl() {
  std::vector<std::unique_ptr<FuncBlock>> expressions;
  optional_unique_ptr<FuncBlock> expr;
  while (stream.PeekCur().type != Token::EndOfFile &&
        (expr = ParseFuncBlock()).has_value()) {
    expressions.push_back(std::move(*expr));
  }

  if (!expr) return std::nullopt;
  LineRange range = FindRangeOfVector(expressions);
  return std::make_unique<FileDecl>(range, std::move(expressions));
}

optional_unique_ptr<FuncBlock> Parser::ParseFuncBlock() {
  Token tok = stream.PeekCur();
  LineRange start = tok.pos;
  optional_unique_ptr<FuncBlock> expr;
  bool ret = false;
  if (tok.type == Token::Return) {
    ret = true;
    stream.PopCur();
    tok = stream.PeekCur();
  }

  // @BUG: when return is the last token it'll just ignore it
  //       rather than raising an error
  if (tok.type == Token::EndOfFile) return std::nullopt;

  if (tok.type == Token::Const || tok.type == Token::Mut) {
    // var decl
    expr = ComposeExpr<FuncBlock>(ParseVarDecl());
  } else if (tok.type == Token::Func) {
    expr = ComposeExpr<FuncBlock>(ParseFuncDecl());
  } else if (tok.type == Token::Struct) {
    expr = ComposeExpr<FuncBlock>(ParseStructDecl());
  } else {
      std::vector<std::unique_ptr<Expr>> lhs;
      if (!ParseList(&Parser::ParseExpr, PushBack, lhs)) return std::nullopt;

      // it could be that it was just an expression so check if next is operator
      // or that we had more than one
      if (lhs.size() > 1 || stream.PeekCur().IsAssignmentOp()) {
        std::vector<std::unique_ptr<Expr>> rhs;
        if (!ParseList(&Parser::ParseExpr, PushBack, rhs)) return std::nullopt;

        auto assign_op = TryTokenCast<AssignmentOp>(stream.PopCur());
        if (!assign_op) return std::nullopt;
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
  }

  if (expr && ret) {
    if ((*expr)->ret) {
      err.ReportCustomErr(DoubleReturnErrMsg, LineRange(start,(*expr)->pos),
                          ErrStream::LexicalErr);
      // we are still fine in this case so meh
    }
    (*expr)->ret = ret;
  }
  return expr;
}

optional_unique_ptr<VarDecl> Parser::ParseFuncDecl() {
  if (!ConsumeToken(Token::Func)) return std::nullopt;

  Token tok = stream.PopCur();
  if (tok.type != Token::Identifier) {
    err.ReportUnexpectedToken(Token::Identifier, tok);
    return std::nullopt;
  }

  auto id = *tok.ToLineStr();
  auto tuple_decl = ParseTupleDecl();
  if (!tuple_decl) return std::nullopt;

  std::optional<std::unique_ptr<TypeExpr>> ret_type = std::nullopt;
  if (stream.PeekCur().type != Token::LeftBrace) {
    auto type_expr = ParseTypeExpr();
    if (!type_expr) return std::nullopt;
    ret_type = std::move(*type_expr);
  }

  auto expr = ParseExpr();
  if (!expr) return std::nullopt;
  LineRange pos = LineRange((*tuple_decl)->pos, (*expr)->pos);
  auto tmp = std::make_unique<Expr>(pos, std::move(*tuple_decl), 
                                    std::move(*ret_type),
                                    std::move(*expr));
  std::vector<VarDecl::Declaration> decl;
  decl.push_back(VarDecl::Declaration(id, std::nullopt, std::move(tmp)));
  return std::make_unique<VarDecl>(pos, true, std::move(decl));
}

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
         std::make_unique<EqualityExpr>(pos,
         std::make_unique<RelationalExpr>(pos,
         std::make_unique<AdditiveExpr>(pos,
         std::make_unique<MultiplicativeExpr>(pos,
         std::make_unique<PowerExpr>(pos,
         std::make_unique<UnaryExpr>(pos,
         std::make_unique<PostfixExpr>(pos, id))))))))));
}

std::unique_ptr<Expr> Parser::ParenthesiseExpr(std::unique_ptr<Expr> expr) {
  LineRange pos = expr->pos;
  return std::make_unique<Expr>(pos,
         std::make_unique<LogicalOrExpr>(pos,
         std::make_unique<LogicalAndExpr>(pos,
         std::make_unique<EqualityExpr>(pos,
         std::make_unique<RelationalExpr>(pos,
         std::make_unique<AdditiveExpr>(pos,
         std::make_unique<MultiplicativeExpr>(pos,
         std::make_unique<PowerExpr>(pos,
         std::make_unique<UnaryExpr>(pos,
         std::make_unique<PostfixExpr>(pos, std::move(expr)))))))))));
}

std::optional<std::vector<LineStr>>
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

  return ids;
}

optional_unique_ptr<VarDecl> Parser::ParseStructDecl() {
  if (!ConsumeToken((Token::Struct))) return std::nullopt;

  Token tok = stream.PopCur();
  if (tok.type != Token::Identifier) {
    err.ReportUnexpectedToken(Token::Identifier, tok);
    return std::nullopt;
  }

  auto id = *tok.ToLineStr();
  auto tuple_decl = ParseTupleDecl();
  if (!tuple_decl) return std::nullopt;

  if (!ConsumeToken((Token::LeftBrace))) return std::nullopt;

  optional_unique_ptr<StructBlock> expr;
  std::vector<std::unique_ptr<StructBlock>> exprs;

  while (stream.PeekCur().type != Token::RightBrace &&
        (expr = ParseStructBlock()).has_value()) {
    exprs.push_back(std::move(*expr));
  }

  if (!ConsumeToken((Token::RightBrace))) return std::nullopt;

  LineRange pos = LineRange((*tuple_decl)->pos, FindRangeOfVector(exprs));
  auto tmp = std::make_unique<Expr>(pos, std::move(*tuple_decl),
                                    std::move(exprs));
  std::vector<VarDecl::Declaration> decl;
  decl.push_back(VarDecl::Declaration(id, std::nullopt, std::move(tmp)));
  return std::make_unique<VarDecl>(pos, true, std::move(decl));
}

optional_unique_ptr<StructBlock> Parser::ParseStructBlock() {
  Token tok = stream.PeekCur();
  if (tok.type == Token::EndOfFile)
    return std::nullopt;

  switch (tok.type) {
    case Token::Func: {
      return ComposeExpr<StructBlock>(ParseFuncDecl());
    }
    case Token::Struct: {
      return ComposeExpr<StructBlock>(ParseStructDecl());
    }
    case Token::Macro: {
      auto expr = ComposeExpr<StructBlock>(ParseMacroExpr());
      if (!ConsumeToken((Token::SemiColon))) return std::nullopt;
      return expr;
    }
    default: {
      auto var_decl = ComposeExpr<StructBlock>(ParseVarDecl());
      if (!ConsumeToken((Token::SemiColon))) return std::nullopt;
      return var_decl;
    } break;
  }

  Unreachable("COMPILER ERROR: Unreachable reached");
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
          (expr = ParseStructBlock()).has_value()) {
      expressions.push_back(std::move(*expr));
    }
    if (!expr) return std::nullopt;
    if (!ConsumeToken((Token::RightBrace))) return std::nullopt;

    LineRange range = FindRangeOfVector(expressions);
    return std::make_unique<Expr>(range, std::move(tuple_decl),
                                  std::move(expressions));
  } else if (tok.type == Token::FatArrow) {
    stream.PopCur();
    auto block = ParseExpr();
    if (!block) return std::nullopt;
    return std::make_unique<Expr>((*block)->pos, std::move(tuple_decl),
                                  std::nullopt,  std::move(*block));
  } else {
    // has to be function just with a type
    auto ret = ParseTypeExpr();
    if (!ret) return std::nullopt;
    if (!ConsumeToken((Token::FatArrow))) return std::nullopt;
    auto block = ParseExpr();
    if (!block) return std::nullopt;
    return std::make_unique<Expr>((*block)->pos, std::move(tuple_decl),
                                  std::move(*ret), std::move(*block));
  }
}

std::optional<TupleDecl::ArgDecl> Parser::ParseTupleDeclSegment() {
  Token tok = stream.PopCur();
  bool is_mut = false;
  bool is_generic = false;
  if (tok.type == Token::Mut) {
    is_mut = true;
    tok = stream.PopCur();
  }

  if (tok.type == Token::Generic) {
    is_generic = true;
    tok = stream.PopCur();
  }

  if (tok.type == Token::Identifier) {
    auto id = *tok.ToLineStr();
    std::optional<std::unique_ptr<TypeExpr>> type_expr = std::nullopt;
    tok = stream.PopCur();
    if (tok.type == Token::Colon) {
      auto type = ParseTypeExpr();
      if (!type) return std::nullopt;
      type_expr = std::move(*type);
      tok = stream.PopCur();
    }

    std::optional<std::unique_ptr<Expr>> val = std::nullopt;
    if (tok.type == Token::Assign) {
      auto expr = ParseExpr();
      if (!expr) return std::nullopt;
      val = std::move(*expr);
      tok = stream.PopCur();
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
                                       ErrStream::LexicalErr);

  if (!ConsumeToken(Token::RightParen)) return std::nullopt;

  // @TODO: fix this
  LineRange pos = LineRange();
  return std::make_unique<TupleDecl>(pos, std::move(declarations));
}

optional_unique_ptr<Expr> Parser::ParseExprOrTupleDecl() {
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
                              ErrStream::LexicalErr);
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
  }

  if (!ConsumeToken((Token::RightParen))) return std::nullopt;

  tok = stream.PeekCur();
  if (tok.type == Token::DoubleColon ||
      tok.type == Token::FatArrow) {
    if (expressions.size() != 0) {
      err.ReportInvalidToken(tok);
      return std::nullopt;
    }
    if (comma) err.ReportCustomErr("Extra ','", prev_tok.pos,
                                   ErrStream::LexicalErr);
    // @FIXME: Fix this
    LineRange pos = LineRange();
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
                          ErrStream::LexicalErr);
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
      if (next.type == Token::Mut) {
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
        Token last = stream.PeekCur();
        while (stream.PeekCur().type != Token::RightParen) {
          comma = false;
          auto expr = ParseExpr();
          if (!expr) return std::nullopt;
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
        }
        if (!ConsumeToken(Token::RightParen)) return std::nullopt;

        if (!comma && exprs.size() == 1) {
          expr = std::move(exprs[0]);
        } else {
          LineRange pos = FindRangeOfVector(exprs);
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
      std::vector<std::unique_ptr<FuncBlock>> exprs;
      stream.PopCur();
      optional_unique_ptr<FuncBlock> func_expr;
      while (stream.PeekCur().type != Token::RightBrace &&
            (func_expr = ParseFuncBlock()).has_value()) {
        exprs.push_back(std::move(*func_expr));
      }
      if (!func_expr) return std::nullopt;
      LineRange pos = FindRangeOfVector(exprs);
      expr = std::make_unique<Expr>(pos, std::move(exprs));
    } break;
    case Token::If: expr = ComposeExpr<Expr>(ParseIfBlock(), true); break;
    case Token::For: expr = ComposeExpr<Expr>(ParseForBlock(), true); break;
    case Token::While: expr = ComposeExpr<Expr>(ParseWhileBlock(), true); break;
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
        expr = std::make_unique<Expr>((*expr)->pos, std::move(*logical_expr));
      }
    } break;
  }

  return expr;
}

optional_unique_ptr<TupleDecl> Parser::ParseTupleDecl() {
  if (!ConsumeToken((Token::LeftParen))) return std::nullopt;
  return ParseRestTupleDeclExpr(std::vector<TupleDecl::ArgDecl>());
}

optional_unique_ptr<VarDecl>
    Parser::ParseVarDeclWithDeclList(std::vector<VarDecl::Declaration> decls) {
  bool atleast_one = false;
  Token cur = stream.PopCur();
  LineRange start = cur.pos;
  if (cur.type == Token::Colon) {
    atleast_one = true;
    // type-expr list
    auto for_each_type = +[](int index, std::unique_ptr<TypeExpr> type_expr, 
                             std::vector<VarDecl::Declaration> &decls) {
      decls.at(index).type = std::move(type_expr);
    };
    if (auto err = ParseList(&Parser::ParseTypeExpr, for_each_type, decls))
      return std::nullopt;
    if (stream.PeekCur().type == Token::Assign) cur = stream.PopCur();
  }

  if (cur.type == Token::Assign) {
    atleast_one = true;
    // expr list
    auto for_each_expr = +[](int index, std::unique_ptr<Expr> expr, 
                             std::vector<VarDecl::Declaration> &decls) {
      decls.at(index).expr = std::move(expr);
    };
    if (auto err = ParseList(&Parser::ParseExpr, for_each_expr, decls))
      return std::nullopt;
  }

  if (!atleast_one) {
    // invalid var_decl
    err.ReportCustomErr("Was expecting either ':' or '=' or both.", cur.pos, 
                        ErrStream::SyntaxErr);
    return std::nullopt;
  }

  LineRange end = stream.PeekCur().pos;
  return std::make_unique<VarDecl>(LineRange(start, end),
                                   std::move(decls));
}

optional_unique_ptr<VarDecl> Parser::ParseVarDecl() {
  // @TODO: very clear that we will get bad errors with this!!
  //        maybe we should rather state that a token is unexpected
  //        along with stating that we were expecting a certain token
  //        rather than stating that a token is 'missing'

  // @NOTE: we need to check for mut/const...?
  // I don't really know what this is doing seems very odd

  // Parse identifier list
  std::vector<VarDecl::Declaration> declarations;
  Token cur = stream.PopCur();
  LineRange start = cur.pos;

  while ((cur = stream.PopCur()).type != Token::Colon &&
         cur.type != Token::Assign) {
    if (cur.type != Token::Identifier) {
      err.ReportUnexpectedToken(Token::Identifier, cur);
      return std::nullopt;
    }

    auto id = *cur.ToLineStr();
    declarations.push_back(VarDecl::Declaration(id,
                                                std::nullopt, std::nullopt));
  }

  return ParseVarDeclWithDeclList(std::move(declarations));
}

std::optional<IfBlock::IfStatement> Parser::ParseIfStatement() {
  if (!ConsumeToken((Token::If))) return std::nullopt;

  auto cond = ParseExpr();
  if (!cond) return std::nullopt;

  auto expr = ParseFuncBlock();
  if (!expr) return std::nullopt;
  return IfBlock::IfStatement(std::move(*cond), std::move(*expr));
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
      auto expr = ParseExpr();
      if (!expr) return std::nullopt;
      LineRange pos = LineRange(statements[0]->pos, (*expr)->pos);
      return std::make_unique<IfBlock>(pos, std::move(statements),
                                       std::move(*expr));
    }
  }
  LineRange pos = LineRange(statements[0]->pos,
                            statements[statements.size() - 1]->pos);
  return std::make_unique<IfBlock>(pos, std::move(statements));
}

optional_unique_ptr<WhileBlock> Parser::ParseWhileBlock() {
  if (!ConsumeToken(Token::While)) return std::nullopt;

  auto cond = ParseExpr();
  if (!cond) return std::nullopt;

  auto expr = ParseExpr();
  if (!expr) return std::nullopt;

  LineRange pos = LineRange((*cond)->pos, (*expr)->pos);
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

  // the assignments i.e. for x, y in z => x, y are the idents
  // @QUESTION: arguably we should also support the `const` / type modifiers
  //            on these but I haven't decided to keep const yet so I want
  //            to decide on that before I impact this
  // @FIXME: decide
  std::vector<LineStr> idents;
  while (stream.PeekCur().type == Token::Identifier) {
    Token tok = stream.PopCur();
    idents.push_back(*tok.ToLineStr());
  }

  if (!ConsumeToken((Token::In))) return std::nullopt;

  std::vector<std::unique_ptr<Expr>> exprs;
  auto push_back = [&exprs](int index, std::unique_ptr<Expr> expr) {
    exprs.push_back(std::move(expr));
  };

  if (auto err = ParseList(&Parser::ParseExpr, push_back))
    return std::nullopt;

  if (consumed_lparen && !ConsumeToken(Token::RightParen)) {
    return std::nullopt;
  }

  auto expr = ParseExpr();
  if (!expr) return std::nullopt;

  LineRange pos = LineRange(start, (*expr)->pos);
  return std::make_unique<ForBlock>(pos, std::move(idents), std::move(*exprs),
                                    std::move(*expr));
}

optional_unique_ptr<FuncCall> Parser::ParseFuncCall() {
  auto func = ParsePostfixExpr();
  if (!func) return std::nullopt;

  if (!ConsumeToken(Token::LeftParen)) return std::nullopt;

  // parse arguments
  std::vector<std::unique_ptr<Expr>> args;
  auto push_back = [&args](int index, std::unique_ptr<Expr> expr) {
    args.push_back(std::move(expr));
  };

  if (auto err = ParseList(&Parser::ParseExpr, push_back))
    return std::nullopt;

  if (!ConsumeToken(Token::RightParen)) return std::nullopt;

  LineRange pos = LineRange((*func)->pos, args.back()->pos);
  return std::make_unique<FuncCall>(pos, std::move(*func), std::move(args));
}

optional_unique_ptr<Constant> Parser::ParseConstant() {
  // can be float, int, string, char, or bool
  // luckily for us this should already be parsed except for bool
  Token tok = stream.PopCur();
  // @SPEED we should be able to make this even faster!!
  // by just forwarding on the data for all cases but bool where we need
  // to just evaluate
  switch (tok.type) {
    case Token::Int: {
      return std::make_unique<Constant>(tok.pos, std::get<std::int64_t>(tok.data));
    } break;
    case Token::Flt: {
      return std::make_unique<Constant>(tok.pos, std::get<double>(tok.data));
    } break;
    case Token::Str: {
      return std::make_unique<Constant>(tok.pos, std::get<LineStr>(tok.data));
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
      err.ReportCustomErr("Invalid constant", tok.pos, ErrStream::SemanticErr);
      return std::nullopt;
    }
  }
}

optional_unique_ptr<MacroExpr> Parser::ParseMacroExpr() {
  stream.PopCur(); // consume the '@'
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
  if (tok.type == Token::LeftParen) {
    // tuple
    // could be empty, void, a series of typed tuple args
    // an can be a subset of that
    stream.PopCur();
    tok = stream.PeekCur();
    if (tok.type == Token::RightParen || tok.type == Token::Void) {
      // empty tuple
      if (!ConsumeToken((Token::RightParen))) return std::nullopt;
      return std::make_unique<TypeExpr>(LineRange(start, tok.pos),
                                        TypeExpr::TupleType());
    }

  } else if (tok.type == Token::Func) {
    // fn pointer/signature

  } else {
    // identifier access or it is an actual expression
  }
}

optional_unique_ptr<AssignmentExpr> Parser::ParseAssignmentExpr() {
  Unreachable("TODO");
}

optional_unique_ptr<Expr::MapExpr> Parser::ParseMapExpr() {
  Unreachable("TODO");
}

optional_unique_ptr<Expr::CollectionExpr> Parser::ParseArrayExpr() {
  Unreachable("TODO");
}

optional_unique_ptr<Expr::CollectionExpr> Parser::ParseTupleExpr() {
  Unreachable("TODO");
}

optional_unique_ptr<PostfixExpr> Parser::ParsePostfixExpr() {
  Unreachable("TODO");
}

optional_unique_ptr<UnaryExpr> Parser::ParseUnaryExpr() {
  Unreachable("TODO");
}

optional_unique_ptr<PowerExpr> Parser::ParsePrefixExpr() {
  Unreachable("TODO");
}

optional_unique_ptr<MultiplicativeExpr> Parser::ParseMultiplicativeExpr() {
  Unreachable("TODO");
}

optional_unique_ptr<AdditiveExpr> Parser::ParseAdditiveExpr() {
  Unreachable("TODO");
}

optional_unique_ptr<RelationalExpr> Parser::ParseRelationalExpr() {
  Unreachable("TODO");
}

optional_unique_ptr<EqualityExpr> Parser::ParseEqualityExpr() {
  Unreachable("TODO");
}

optional_unique_ptr<LogicalAndExpr> Parser::ParseLogicalAndExpr() {
  Unreachable("TODO");
}

optional_unique_ptr<LogicalOrExpr> Parser::ParseLogicalOrExpr() {
  Unreachable("TODO");
}

}