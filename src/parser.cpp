#include "parser.hpp"

#include <rang.hpp>

#include "helper.hpp"

namespace porc::internals {

bool Parser::ConsumeToken(Token wanted) {
  Token tok = stream.PeekCur();
  // check if next token is what we want
  if (tok.type == Token::EndOfFile) {
    err.ReportExpectedToken(wanted, tok.pos);
    return false;
  } else if (tok.type != wanted.type) {
    err.ReportUnexpectedToken(wanted, tok);
    return false;
  }
  stream.PopCur(); // actually consume token
  return true;
}

template<typename T>
LineRange FindRangeOfVector(std::vector<std::unique_ptr<T>> &vec) {
  switch (vec.size()) {
    case 0: return LineRange::Null();
    case 1: return vec.at(0)->pos;
    default: return LineRange(vec.at(0)->pos, vec.at(vec.size() - 1)->pos);
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
  std::vector<std::unique_ptr<FileBlock>> expressions;
  optional_unique_ptr<FileBlock> expr;
  while (stream.PeekCur().type != Token::EndOfFile &&
        (expr = ParseFileBlock()).has_value()) {
    expressions.push_back(std::move(*expr));
  }

  if (!expr) return unexpected(expr.error());
  LineRange range = FindRangeOfVector(expressions);
  return std::make_unique<FileDecl>(range, std::move(expressions));
}

optional_unique_ptr<FuncBlock> Parser::ParseFuncBlock() {
  Token tok = stream.PopCur();
  if (tok.type == Token::Return) {
    auto expr = ComposeExpr<FuncBlock>(ParseExpr(), /* return: */ true);
    if (!ConsumeToken(Token::SemiColon))
      return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "Missing semicolon", Token::SemiColon));
    return expr;
  }

  // We want to look at the tokens and try to guess accurately
  // whether or not it is an assignment, expr, or a var_decl
  Token next = stream.PeekCur();
  if (tok.type == Token::Const || (tok.type == Token::Identifier &&
                                        (next.type == Token::Colon ||
                                         next.type == Token::Assign))) {
    // easiest case has to be a var decl
    stream.Push(tok);
    auto expr = ComposeExpr<FuncBlock>(ParseVarDecl(), false);
    if (!ConsumeToken(Token::SemiColon))
      return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "Missing semicolon", Token::SemiColon));
    return expr;
  }

  if (tok.type == Token::Identifier) {
    // could be either var decl/expr (if it just consists of an Ident)
    // or assign_expr
    stream.Push(tok);
    if (next.type != Token::Comma && !next.IsAssignmentOp()) {
      bool ret = next.type != Token::SemiColon;
      auto expr = ComposeExpr<FuncBlock>(ParseExpr(), ret);
      if (!ret) stream.PopCur();
      return expr;
    }

    auto list = ParseAssignmentIdentifierList(Token::Comma);
    if (!list) return unexpected(list.error());

    tok = stream.PeekCur();
    if (tok.type == Token::Colon) {
      if (auto ids = std::get_if<std::vector<VarDecl::Declaration>>(&*list)) {
        auto expr = ComposeExpr<FuncBlock>(ParseVarDeclWithDeclList(std::move(*ids)), false);
        if (!ConsumeToken(Token::SemiColon))
          return unexpected(ParseError(ParseError::Kind::MissingToken,
                                       "Missing semicolon", Token::SemiColon));
        return expr;
      } else {
        return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                     "Unexpected Colon Token", tok));
      }
    }

    auto assign_op = TryTokenCast<AssignmentOp>(stream.PopCur());
    if (!assign_op) return unexpected(assign_op.error());
    LineRange pos = LineRange::Null();

    if (*assign_op != AssignmentOp::Kind::Equal ||
        std::holds_alternative<std::vector<std::unique_ptr<Expr>>>(*list)) {
      std::vector<std::unique_ptr<Expr>> lhs;
      if (auto ids = std::get_if<std::vector<VarDecl::Declaration>>(&*list)) {
        for (auto &id : *ids) lhs.push_back(ConvIdentToExpr(id.id, pos));
      } else {
        lhs = std::move(std::get<std::vector<std::unique_ptr<Expr>>>(*list));
      }

      std::vector<std::unique_ptr<Expr>> rhs;
      auto push_back = [&rhs](int index, std::unique_ptr<Expr> expr) {
        rhs.push_back(std::move(expr));
      };

      if (auto err = ParseList(&Parser::ParseExpr, push_back))
        return unexpected(*err);
      auto assign_expr = std::make_unique<AssignmentExpr>(pos, std::move(lhs),
                                                          *assign_op, 
                                                          std::move(rhs));
      return std::make_unique<FuncBlock>(pos, std::move(assign_expr));
    } else {
      auto ids = std::get<std::vector<VarDecl::Declaration>>(std::move(*list));
      auto push_back = [&ids](int index, std::unique_ptr<Expr> expr) {
        ids.at(index).expr = std::move(expr);
      };
      if (auto err = ParseList(&Parser::ParseExpr, push_back))
        return unexpected(*err);
      auto var_decl = std::make_unique<VarDecl>(pos, std::move(ids));
      return std::make_unique<FuncBlock>(pos, std::move(var_decl), false);
    }
  } else {
    // can't be var decl
    LineRange start = tok.pos;
    stream.Push(tok);
    // either expr or assignment expr both still have an expr as first arg
    std::vector<std::unique_ptr<Expr>> list;
    // find better way of expressing this lambda
    auto push_back = [&list](int index, std::unique_ptr<Expr> obj) {
      list.push_back(std::move(obj));
    };
    if (auto err = ParseList(&Parser::ParseExpr, push_back))
      return unexpected(*err);
    tok = stream.PeekCur();
    if (tok.IsAssignmentOp()) {
      auto assign_op = TryTokenCast<AssignmentOp>(stream.PopCur());
      if (!assign_op) return unexpected(assign_op.error());
      std::vector<std::unique_ptr<Expr>> lhs = std::move(list);
      LineRange end = lhs.at(lhs.size() - 1)->pos;
      LineRange pos = LineRange(start, end);

      if (auto err = ParseList(&Parser::ParseExpr, push_back))
        return unexpected(*err);
      auto assign_expr = std::make_unique<AssignmentExpr>(pos, std::move(lhs),
                                                          *assign_op, 
                                                          std::move(list));
      return std::make_unique<FuncBlock>(pos, std::move(assign_expr));
    } else if (list.size() != 1) {
      return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                   "Was expecting a comma", Token::Comma));
    } else {
      bool ret = tok.type != Token::SemiColon;
      LineRange pos = list.at(0)->pos;
      return std::make_unique<FuncBlock>(pos, std::move(list.at(0)), ret);
    }
  }
}

optional_unique_ptr<VarDecl> Parser::ParseFileFuncDecl() {
  if (!ConsumeToken(Token::Func)) {
    return unexpected(ParseError(ParseError::Kind::MissingToken,
                                 "Missing 'fn'", Token::Func));
  }
  Token tok = stream.PopCur();
  if (tok.type != Token::Identifier) {
    UnexpectedToken(Token::Identifier, tok);
    return unexpected(ParseError(ParseError::Kind::MissingToken,
                                 "Missing identifier", Token::Identifier));
  }

  if (auto id = std::get_if<std::string>(&tok.data)) {
    auto tuple_decl = ParseTupleDecl();
    if (!tuple_decl) return unexpected(tuple_decl.error());

    std::optional<std::unique_ptr<TypeExpr>> ret_type = std::nullopt;
    if (stream.PeekCur().type != Token::LeftBrace) {
      auto type_expr = ParseTypeExpr();
      if (!type_expr) return unexpected(type_expr.error());
      ret_type = std::move(*type_expr);
    }

    auto expr = ParseExpr();
    if (!expr) return unexpected(expr.error());
    LineRange pos = LineRange((*tuple_decl)->pos, (*expr)->pos);
    auto tmp = std::make_unique<Expr>(pos, std::move(*tuple_decl), 
                                      std::move(*ret_type),
                                      std::move(*expr));
    std::vector<VarDecl::Declaration> decl;
    decl.push_back(VarDecl::Declaration(true, *id, std::move(tmp)));
    return std::make_unique<VarDecl>(pos, std::move(decl));
  } else {
    return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                 "Identifier is missing", tok));
  }
}

/*
  The following is ugly ish
  It is mostly due to me having to do a bad conversion
  This is kinda rare and fine, a slightly better way could be
  to have this kind of conversion inside the AST that is allow the ast
  to have another option that jumps from expr -> postfix

  Also note: that in this case it is doing the same conversion really
*/

std::unique_ptr<Expr> Parser::ConvIdentToExpr(std::string id, LineRange pos) {
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

expected<std::variant<std::vector<VarDecl::Declaration>,
         std::vector<std::unique_ptr<Expr>>>, ParseError>
    Parser::ParseAssignmentIdentifierList(Token::Kind continuer) {
  std::vector<VarDecl::Declaration> ids;
  LineRange start = stream.PeekCur().pos;
  Token tok;
  bool any_const = false;

  while (true) {
    tok = stream.PeekCur();
    bool is_const = false;
    if (tok.type == Token::Const) {
      is_const = any_const = true;
      stream.PopCur();
      tok = stream.PeekCur();
    }

    if (tok.type != Token::Identifier) {
      if (any_const) return unexpected(ParseError(ParseError::Kind::MissingToken,
                                                 "Was expecting identifier",
                                                 Token::Identifier));

      // we are expecting an identifier so we know it is now an expr list
      // so just convert all our identifiers to expressions and then push them on
      std::vector<std::unique_ptr<Expr>> exprs;
      // we have atleast this 'expr' and all the previous identifiers
      exprs.reserve(ids.size() + 1);
      // note: the position here is wrong!  But we can also just adjust the position
      // as we go but it may still result in being wrong
      // so we really should keep the LineRange along with the string
      // maybe have an identifier struct to use rather than std::string.
      for (auto &id : ids) exprs.push_back(ConvIdentToExpr(id.id, start));
      auto for_each = [&exprs](int index, std::unique_ptr<Expr> expr) {
        exprs.push_back(std::move(expr));
      };
      if (auto err = ParseList(&Parser::ParseExpr, for_each)) return unexpected(*err);
      return exprs;
    }
    stream.PopCur();
    if (auto id = std::get_if<std::string>(&tok.data)) {
      ids.push_back(VarDecl::Declaration(is_const, *id, std::nullopt,
                                         std::nullopt));
    } else {
      return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                   "Identifier is invalid", tok));
    }
    // if there is no dot then this is the end of the identifier access
    if (stream.PeekCur().type != continuer) break;
    stream.PopCur(); // remove the continuer
  }

  return ids;
}

expected<std::vector<std::string>, ParseError>
    Parser::ParseIdentifierAccess(Token::Kind continuer) {
  std::vector<std::string> ids;
  Token tok;
  while (true) {
    tok = stream.PeekCur();
    if (tok.type != Token::Identifier) {
     return unexpected(ParseError(ParseError::Kind::MissingToken,
                                  "Identifier is missing", Token::Identifier));
    }
    stream.PopCur();
    if (auto id = std::get_if<std::string>(&tok.data)) {
      ids.push_back(*id);
    } else {
      return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                   "Identifier is invalid", tok));
    }
    // if there is no dot then this is the end of the identifier access
    if (stream.PeekCur().type != continuer) break;
    stream.PopCur(); // remove the continuer
  }

  return ids;
}

optional_unique_ptr<VarDecl> Parser::ParseFileStructDecl() {
  if (!ConsumeToken(Token::Struct)) {
      return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "Missing 'struct'", Token::Struct));
  }

  Token tok = stream.PopCur();
  if (tok.type != Token::Identifier) {
    UnexpectedToken(Token::Identifier, tok);
    return unexpected(ParseError(ParseError::Kind::MissingToken,
                                 "Missing identifier", Token::Identifier));
  }

  if (auto id = std::get_if<std::string>(&tok.data)) {
    auto tuple_decl = ParseTupleDecl();
    if (!tuple_decl) return unexpected(tuple_decl.error());

    if (!ConsumeToken(Token::LeftBrace))
      return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "Missing left brace", Token::LeftBrace));

    optional_unique_ptr<FileBlock> expr;
    std::vector<std::unique_ptr<FileBlock>> exprs;

    while (stream.PeekCur().type != Token::RightBrace &&
          (expr = ParseFileBlock()).has_value()) {
      exprs.push_back(std::move(*expr));
    }

    if (!ConsumeToken(Token::RightBrace)) {
      if (!expr) return unexpected(expr.error());
      return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "Missing right brace", Token::RightBrace));
    }

    LineRange pos = LineRange((*tuple_decl)->pos, FindRangeOfVector(exprs));
    auto tmp = std::make_unique<Expr>(pos, std::move(*tuple_decl),
                                      std::move(exprs));
    std::vector<VarDecl::Declaration> decl;
    decl.push_back(VarDecl::Declaration(true, *id, std::move(tmp)));
    return std::make_unique<VarDecl>(pos, std::move(decl));
  } else {
    return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                 "Identifier is missing", tok));
  }
}

optional_unique_ptr<FileBlock> Parser::ParseFileBlock() {
  Token tok = stream.PeekCur();
  if (tok.type == Token::EndOfFile)
    return unexpected(ParseError(ParseError::Kind::ValidEOF));

  switch (tok.type) {
    case Token::Func: {
      return ComposeExpr<FileBlock>(ParseFileFuncDecl());
    }
    case Token::Struct: {
      return ComposeExpr<FileBlock>(ParseFileStructDecl());
    }
    case Token::Macro: {
      auto expr = ComposeExpr<FileBlock>(ParseMacroExpr());
      if (!ConsumeToken(Token::SemiColon))
        return unexpected(ParseError(ParseError::Kind::MissingToken,
                                     "Missing semicolon", Token::SemiColon));
      return expr;
    }
    default: {
      auto var_decl = ComposeExpr<FileBlock>(ParseVarDecl());
      if (!ConsumeToken(Token::SemiColon))
        return unexpected(ParseError(ParseError::Kind::MissingToken,
                                     "Missing semicolon", Token::SemiColon));
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
    if (!ConsumeToken(Token::LeftBrace))
      return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "Missing '{'", Token::LeftBrace));
    std::vector<std::unique_ptr<FileBlock>> expressions;
    optional_unique_ptr<FileBlock> expr;
    while (stream.PeekCur().type != Token::RightBrace &&
          (expr = ParseFileBlock()).has_value()) {
      expressions.push_back(std::move(*expr));
    }
    if (!expr) return unexpected(expr.error());
    if (!ConsumeToken(Token::RightBrace))
      return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "Missing '}'", Token::RightBrace));

    LineRange range = FindRangeOfVector(expressions);
    return std::make_unique<Expr>(range, std::move(tuple_decl),
                                  std::move(expressions));
  } else if (tok.type == Token::FatArrow) {
    stream.PopCur();
    auto block = ParseExpr();
    if (!block) return unexpected(block.error());
    return std::make_unique<Expr>((*block)->pos, std::move(tuple_decl),
                                  std::nullopt,  std::move(*block));
  } else {
    // has to be function just with a type
    auto ret = ParseTypeExpr();
    if (!ret) return unexpected(ret.error());
    if (!ConsumeToken(Token::FatArrow))
      return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "Missing '=>'", Token::FatArrow));
    auto block = ParseExpr();
    if (!block) return unexpected(block.error());
    return std::make_unique<Expr>((*block)->pos, std::move(tuple_decl),
                                  std::move(*ret), std::move(*block));
  }
}

expected<TupleDecl::ArgDecl, ParseError> Parser::ParseTupleDeclSegment() {
  Token tok = stream.PopCur();
  bool is_const = false;
  if (tok.type == Token::Const) {
    is_const = true;
    tok = stream.PopCur();
  }

  if (tok.type == Token::Identifier) {
    if (auto id = std::get_if<std::string>(&tok.data)) {
      std::optional<std::unique_ptr<TypeExpr>> type_expr = std::nullopt;
      tok = stream.PopCur();
      if (tok.type == Token::Colon) {
        auto type = ParseTypeExpr();
        if (!type) return unexpected(type.error());
        type_expr = std::move(*type);
        tok = stream.PopCur();
      }

      std::optional<std::unique_ptr<Expr>> val = std::nullopt;
      if (tok.type == Token::Assign) {
        auto expr = ParseExpr();
        if (!expr) return unexpected(expr.error());
        val = std::move(*expr);
        tok = stream.PopCur();
      }

      return TupleDecl::ArgDecl(is_const, std::move(*id), std::move(type_expr),
                                std::move(val));
    } else {
      return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                   "Identifier is invalid", tok));
    }
  } else {
    return unexpected(ParseError(ParseError::Kind::MissingToken,
                                 "Missing identifier", Token::Identifier));
  }
}

optional_unique_ptr<TupleDecl> Parser::ParseRestTupleDeclExpr(
    std::vector<TupleDecl::ArgDecl> declarations) {
  // presuming atleast the '(' has been consumed and all has been correctly
  // parsed into declarations
  bool extra_comma = false;
  while (stream.PeekCur().type != Token::RightParen) {
    extra_comma = false;
    auto decl = ParseTupleDeclSegment();
    if (!decl) return unexpected(decl.error());
    declarations.push_back(std::move(*decl));

    Token tok = stream.PeekCur();
    if (tok.type != Token::Comma) break;
    stream.PopCur(); // pop comma
    extra_comma = true;
  }

  if (extra_comma) return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                                "Extra ','", Token::Comma));

  if (!ConsumeToken(Token::RightParen))
    return unexpected(ParseError(ParseError::Kind::MissingToken,
                                 "Missing ')'", Token::RightParen));

  LineRange pos = LineRange::Null();
  return std::make_unique<TupleDecl>(pos, std::move(declarations));
}

optional_unique_ptr<Expr> Parser::ParseExprOrTupleDecl() {
  std::vector<TupleDecl::ArgDecl> declarations;
  bool comma = false;
  std::vector<std::unique_ptr<Expr>> expressions;

  Token tok = stream.PeekCur();

  while (tok.type != Token::RightParen) {
    comma = false;
    tok = stream.PopCur();

    if (tok.type == Token::Const) {
      stream.Push(tok);
      auto tuple_decl = ParseRestTupleDeclExpr(std::move(declarations));
      if (!tuple_decl) return unexpected(tuple_decl.error());
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
      if (!decl) return unexpected(decl.error());
      bool is_tuple_decl = decl->type || decl->expr;
      declarations.push_back(std::move(*decl));

      if (stream.PeekCur().type == Token::Comma) {
        comma = true;
        stream.PopCur();
      }

      if (is_tuple_decl) {
        if (comma && stream.PeekCur().type == Token::RightParen) {
          return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                        "Extra ','", Token::Comma));
        }

        auto tuple_decl = ParseRestTupleDeclExpr(std::move(declarations));
        if (!tuple_decl) return unexpected(tuple_decl.error());
        return ParseExprFuncOrStruct(std::move(*tuple_decl));
      }
    } else {
      // expression
      stream.Push(tok);
      std::for_each(declarations.cbegin(), declarations.cend(),
        [&expressions, this](auto &decl) {
          expressions.push_back(ConvIdentToExpr(decl.id, LineRange::Null()));
        });

      optional_unique_ptr<Expr> expr;
      while (stream.PeekCur().type != Token::RightParen &&
            (expr = ParseExpr()).has_value()) {
        comma = false;
        expressions.push_back(std::move(*expr));
        if (stream.PeekCur().type != Token::Comma) break;
        stream.PopCur(); // pop comma
        comma = true;
      }

      if (!expr) return unexpected(expr.error());
      break;
    }
  }

  if (!ConsumeToken(Token::RightParen))
    return unexpected(ParseError(ParseError::Kind::MissingToken,
                                 "Missing ')'", Token::RightParen));

  tok = stream.PeekCur();
  if (tok.type == Token::DoubleColon ||
      tok.type == Token::FatArrow) {
    if (expressions.size() != 0)
      return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                   "Wasn't expecting => or ::", tok));
    if (comma) return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                            "Extra ','", Token::Comma));
    LineRange pos = LineRange::Null();
    return ParseExprFuncOrStruct(std::make_unique<TupleDecl>(pos,
      std::move(declarations)));
  }

  // convert all declarations into expressions anyway
  if (expressions.size() == 0) {
      std::for_each(declarations.cbegin(), declarations.cend(),
        [&expressions, this](auto &decl) {
          expressions.push_back(ConvIdentToExpr(decl.id, LineRange::Null()));
        });
  }

  if (expressions.size() == 1 && !comma) {
    // literally just the first expression wrapped in paren
    return ParenthesiseExpr(std::move(expressions[0]));
  } else {
    if (expressions.size() != 1 && comma)
      return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                   "Extra ','", Token::Comma));
    // tuple
    LineRange pos = FindRangeOfVector(expressions);
    return std::make_unique<Expr>(pos, std::move(expressions), false);
  }
}

optional_unique_ptr<Expr> Parser::ParseExpr() {
  switch (stream.PeekCur().type) {
    case Token::Let: {
      stream.PopCur();
      return ComposeExpr<Expr>(ParseVarDecl());
    }
    case Token::LeftParen: {
      // tuple/func/struct or simply '(' expr ')'
      Token tok = stream.PopCur();
      Token next = stream.PeekCur();
      if (next.type == Token::Const) {
        // either func/struct
        stream.Push(tok);
        auto tuple_decl = ParseTupleDecl();
        if (!tuple_decl) return unexpected(tuple_decl.error());
        return ParseExprFuncOrStruct(std::move(*tuple_decl));
      } else if (next.type == Token::Identifier) {
        // two trees we care about
        // 1) func/struct where this is a tuple decl
        // 2) tuple/expr where this is well a list of expressions
        stream.Push(tok);
        return ParseExprOrTupleDecl();
      } else {
        // has to be a tuple
        std::vector<std::unique_ptr<Expr>> exprs;
        bool comma = false;
        while (stream.PeekCur().type != Token::RightParen) {
          comma = false;
          auto expr = ParseExpr();
          if (!expr) return unexpected(expr.error());
          if (stream.PeekCur().type == Token::Comma) {
            stream.PopCur();
            comma = true;
          } else if (stream.PeekCur().type != Token::RightParen) {
            return unexpected(ParseError(ParseError::Kind::MissingToken,
                                         "Expecting '(' or ','", Token::Comma));
          }
        }

        if (comma && exprs.size() != 1) {
          return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                       "Extra ','", Token::Comma));
        }
        if (!ConsumeToken(Token::RightParen))
          return unexpected(ParseError(ParseError::Kind::MissingToken,
                                       "Missing ')'", Token::RightParen));
        if (!comma && exprs.size() == 1) {
          return std::move(exprs[0]);
        } else {
          LineRange pos = FindRangeOfVector(exprs);
          return std::make_unique<Expr>(pos, std::move(exprs), false);
        }
      }
    }
    case Token::LeftBracket: {
      // array/map
      auto first_val = ParseExpr();
      if (!first_val) return unexpected(first_val.error());
      if (stream.PeekCur().type == Token::Colon) {
        // we have a map
        std::vector<std::unique_ptr<Expr>> keys;
        std::vector<std::unique_ptr<Expr>> values;
        keys.push_back(std::move(*first_val));
        first_val = ParseExpr();
        if (!first_val) return unexpected(first_val.error());
        values.push_back(std::move(*first_val));
        auto foreach = [&keys, &values](int index, std::unique_ptr<Expr> key,
                                         std::unique_ptr<Expr> val) {
          keys.push_back(std::move(key));
          values.push_back(std::move(val));
        };
        if (auto err = ParseListConjugate(&Parser::ParseExpr, foreach))
          return unexpected(*err);
        LineRange pos = LineRange(keys[0]->pos, values[values.size() - 1]->pos);
        return std::make_unique<Expr>(pos, std::move(keys), std::move(values));
      } else {
        std::vector<std::unique_ptr<Expr>> vals;
        vals.push_back(std::move(*first_val));
        auto foreach = [&vals](int index, std::unique_ptr<Expr> expr) {
          vals.push_back(std::move(expr));
        };
        if (auto err = ParseList(&Parser::ParseExpr, foreach))
          return unexpected(*err);
        LineRange pos = FindRangeOfVector(vals);
        return std::make_unique<Expr>(pos, std::move(vals), true);
      }
    }
    case Token::LeftBrace: {
      std::vector<std::unique_ptr<FuncBlock>> exprs;
      stream.PopCur();
      optional_unique_ptr<FuncBlock> expr;
      while (stream.PeekCur().type != Token::RightBrace &&
            (expr = ParseFuncBlock()).has_value()) {
        exprs.push_back(std::move(*expr));
      }
      if (!expr) return unexpected(expr.error());
      LineRange pos = FindRangeOfVector(exprs);
      return std::make_unique<Expr>(pos, std::move(exprs));
    }
    case Token::If: return ComposeExpr<Expr>(ParseIfBlock());
    case Token::For: return ComposeExpr<Expr>(ParseForBlock());
    case Token::While: return ComposeExpr<Expr>(ParseWhileBlock());
    default: {
      auto expr = ParseLogicalOrExpr();
      if (!expr) return unexpected(expr.error());
      if (stream.PeekCur().type == Token::Range) {
        // range based statement
        stream.PopCur();
        bool inclusive_right = false;
        if (stream.PeekCur().type == Token::Assign) {
          inclusive_right = true;
          stream.PopCur();
        }
        auto stop = ParseLogicalOrExpr();
        if (!stop) return unexpected(stop.error());
        LineRange pos = LineRange((*expr)->pos, (*stop)->pos);
        std::optional<std::unique_ptr<LogicalOrExpr>> step = std::nullopt;
        if (stream.PeekCur().type == Token::Colon) {
          stream.PopCur();
          auto tmp_step = ParseLogicalOrExpr();
          if (!tmp_step) return unexpected(tmp_step.error());
          step = std::move(*tmp_step);
          pos = LineRange(pos, (*step)->pos);
        }
        return std::make_unique<Expr>(pos, inclusive_right, std::move(*expr),
                                      std::move(*stop), std::move(step));
      } else {
        return std::make_unique<Expr>((*expr)->pos, std::move(*expr));
      }
    }
  }

  Unreachable("Case not handled");
}

optional_unique_ptr<TupleDecl> Parser::ParseTupleDecl() {
  if (!ConsumeToken(Token::LeftParen))
    return unexpected(ParseError(ParseError::Kind::MissingToken,
                                "Missing '('", Token::LeftParen));
  return ParseRestTupleDeclExpr(std::vector<TupleDecl::ArgDecl>());
}

optional_unique_ptr<VarDecl>
    Parser::ParseVarDeclWithDeclList(std::vector<VarDecl::Declaration> declarations) {
  bool atleast_one = false;
  Token cur = stream.PopCur();
  LineRange start = cur.pos;
  if (cur.type == Token::Colon) {
    atleast_one = true;
    // type-expr list
    auto for_each_type = [&declarations](int index,
                                         std::unique_ptr<TypeExpr> type_expr) {
      declarations.at(index).type = std::move(type_expr);
    };
    if (auto err = ParseList(&Parser::ParseTypeExpr, for_each_type))
      return unexpected(*err);
    if (stream.PeekCur().type == Token::Assign) cur = stream.PopCur();
  }

  if (cur.type == Token::Assign) {
    atleast_one = true;
    // expr list
    auto for_each_expr = [&declarations](int index,
                                         std::unique_ptr<Expr> expr) {
      declarations.at(index).expr = std::move(expr);
    };
    if (auto err = ParseList(&Parser::ParseExpr, for_each_expr))
      return unexpected(*err);
  }

  if (!atleast_one) {
    // invalid var_decl
    return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                 "Was expecting one of or both of ':'/'='",
                                 Token::Assign));
  }

  LineRange end = stream.PeekCur().pos;
  return std::make_unique<VarDecl>(LineRange(start, end),
                                   std::move(declarations));
}

optional_unique_ptr<VarDecl> Parser::ParseVarDecl() {
  // @TODO: very clear that we will get bad errors with this!!
  //        maybe we should rather state that a token is unexpected
  //        along with stating that we were expecting a certain token
  //        rather than stating that a token is 'missing'

  // Parse identifier list
  std::vector<VarDecl::Declaration> declarations;
  Token cur = stream.PopCur();
  LineRange start = cur.pos;

  while ((cur = stream.PopCur()).type != Token::Colon &&
         cur.type != Token::Assign) {
    bool is_const = false;
    if (cur.type == Token::Const) {
      is_const = true;
      cur = stream.PopCur();
    }
    if (cur.type != Token::Identifier) {
     return unexpected(ParseError(ParseError::Kind::MissingToken,
                                  "Identifier is missing", Token::Identifier));
    }

    if (auto id = std::get_if<std::string>(&cur.data)) {
      declarations.push_back(VarDecl::Declaration(is_const, *id,
                                                  std::nullopt, std::nullopt));
    } else {
      return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                   "Identifier is invalid", cur));
    }
  }

  return ParseVarDeclWithDeclList(std::move(declarations));
}

expected<IfBlock::IfStatement, ParseError> Parser::ParseIfStatement() {
  if (!ConsumeToken(Token::If)) {
      return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "Expected 'if'", Token::If));
  }

  auto cond = ParseExpr();

  auto expr = ParseExpr();
  if (!expr) return unexpected(expr.error());
  return IfBlock::IfStatement(std::move(*cond), std::move(*expr));
}

optional_unique_ptr<IfBlock> Parser::ParseIfBlock() {
  std::vector<IfBlock::IfStatement> statements;
  auto statement = ParseIfStatement();
  if (!statement) return unexpected(statement.error());
  statements.push_back(std::move(*statement));

  while (stream.PeekCur().type == Token::Else) {
    // then either else if or else
    stream.PopCur();
    if (stream.PeekCur().type == Token::If) {
      statement = ParseIfStatement();
      if (!statement) return unexpected(statement.error());
      statements.push_back(std::move(*statement));
    } else {
      // else and then end the function
      auto expr = ParseExpr();
      if (!expr) return unexpected(expr.error());
      return std::make_unique<IfBlock>(LineRange::Null(), std::move(statements),
                                       std::move(*expr));
    }
  }
  return std::make_unique<IfBlock>(LineRange::Null(), std::move(statements));
}

optional_unique_ptr<WhileBlock> Parser::ParseWhileBlock() {
  if (!ConsumeToken(Token::While)) {
      return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "Expected 'while'", Token::While));
  }

  auto cond = ParseExpr();
  if (!cond) return unexpected(cond.error());

  auto expr = ParseExpr();
  if (!expr) return unexpected(expr.error());

  LineRange pos = LineRange((*cond)->pos, (*expr)->pos);
  return std::make_unique<WhileBlock>(pos, std::move(*cond), std::move(*expr));
}

optional_unique_ptr<ForBlock> Parser::ParseForBlock() {
  LineRange start = stream.PeekCur().pos;
  if (!ConsumeToken(Token::For)) {
      return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "Expected 'for'", Token::For));
  }

  // to support parenthesis around the `ident_list list 'in' expr_list`
  // we just do it manually, and track if we consumed the '(' so that we
  // can expect the ')'
  bool consumed_lparen = false;
  if (stream.PeekCur().type == Token::LeftParen) {
    consumed_lparen = true;
    if (!ConsumeToken(Token::LeftParen)) {
      return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "Expected '('", Token::LeftParen));
    }
  }

  // the assignments i.e. for x, y in z => x, y are the idents
  // @QUESTION: arguably we should also support the `const` / type modifiers
  //            on these but I haven't decided to keep const yet so I want
  //            to decide on that before I impact this
  // @FIXME: decide
  std::vector<std::string> idents;
  while (stream.PeekCur().type == Token::Identifier) {
    idents.push_back(std::get<std::string>(stream.PopCur().data));
  }

  if (!ConsumeToken(Token::In)) {
      return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "Expected 'in'", Token::In));
  }

  std::vector<std::unique_ptr<Expr>> exprs;
  auto push_back = [&exprs](int index, std::unique_ptr<Expr> expr) {
    exprs.push_back(std::move(expr));
  };

  if (auto err = ParseList(&Parser::ParseExpr, push_back))
    return unexpected(*err);

  if (consumed_lparen && !ConsumeToken(Token::RightParen)) {
    return unexpected(ParseError(ParseError::Kind::MissingToken,
                                  "Expected ')'", Token::RightParen));
  }

  auto expr = ParseExpr();
  if (!expr) return unexpected(expr.error());

  LineRange pos = LineRange(start, (*expr)->pos);
  return std::make_unique<ForBlock>(pos, std::move(idents), std::move(*exprs),
                                    std::move(*expr));
}

optional_unique_ptr<FuncCall> Parser::ParseFuncCall() {
  auto func = ParsePostfixExpr();
  if (!func) return unexpected(func.error());

  if (!ConsumeToken(Token::LeftParen)) {
    return unexpected(ParseError(ParseError::Kind::MissingToken,
                                  "Expected '('", Token::LeftParen));
  }

  // parse arguments
  std::vector<std::unique_ptr<Expr>> args;
  auto push_back = [&args](int index, std::unique_ptr<Expr> expr) {
    args.push_back(std::move(expr));
  };

  if (auto err = ParseList(&Parser::ParseExpr, push_back))
    return unexpected(*err);

  if (!ConsumeToken(Token::RightParen)) {
    return unexpected(ParseError(ParseError::Kind::MissingToken,
                                  "Expected ')'", Token::RightParen));
  }

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
      return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                   "Was expecting a constant", tok));
    }
  }
}

optional_unique_ptr<MacroExpr> Parser::ParseMacroExpr() {
  stream.PopCur(); // consume the '@'
  Token tok = stream.PopCur();
  LineRange pos_start = tok.pos;
  LineRange pos_end = tok.pos;

  auto ids = ParseIdentifierAccess(Token::Dot);
  if (!ids) return unexpected(ids.error());

  if (!ConsumeToken(Token::LeftParen))
    return unexpected(ParseError(ParseError::Kind::MissingToken,
                                "Missing left paren", Token::LeftParen));

  std::vector<std::unique_ptr<Expr>> exprs;
  while (stream.PeekCur().type != Token::RightParen) {
    auto expr = ParseExpr();
    if (!expr) return unexpected(expr.error());
    exprs.push_back(std::move(*expr));

    tok = stream.PeekCur();
    if (tok.type == Token::Comma) {
      stream.PopCur();
    } else if (tok.type != Token::RightParen) {
      return unexpected(ParseError(ParseError::Kind::InvalidToken,
                                   "Expecting '(' or ','", tok));
    } else {
      pos_end = tok.pos;
    }
  }

  if (!ConsumeToken(Token::RightParen))
    return unexpected(ParseError(ParseError::Kind::MissingToken,
                                 "Missing right paren", Token::RightParen));
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
      if (!ConsumeToken(Token::RightParen))
        return unexpected(ParseError(ParseError::Kind::MissingToken,
                                     "Missing right paren", Token::RightParen));
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
  return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "TODO 'assignexpr'", Token::If));
}

optional_unique_ptr<Expr::MapExpr> Parser::ParseMapExpr() {
  return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "TODO 'assignexpr'", Token::If));
}

optional_unique_ptr<Expr::CollectionExpr> Parser::ParseArrayExpr() {
  return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "TODO 'assignexpr'", Token::If));
}

optional_unique_ptr<Expr::CollectionExpr> Parser::ParseTupleExpr() {
  return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "TODO 'assignexpr'", Token::If));
}

optional_unique_ptr<PostfixExpr> Parser::ParsePostfixExpr() {
  return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "TODO 'assignexpr'", Token::If));
}

optional_unique_ptr<UnaryExpr> Parser::ParseUnaryExpr() {
  return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "TODO 'assignexpr'", Token::If));
}

optional_unique_ptr<PowerExpr> Parser::ParsePrefixExpr() {
  return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "TODO 'assignexpr'", Token::If));
}

optional_unique_ptr<MultiplicativeExpr> Parser::ParseMultiplicativeExpr() {
  return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "TODO 'assignexpr'", Token::If));
}

optional_unique_ptr<AdditiveExpr> Parser::ParseAdditiveExpr() {
  return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "TODO 'assignexpr'", Token::If));
}

optional_unique_ptr<RelationalExpr> Parser::ParseRelationalExpr() {
  return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "TODO 'assignexpr'", Token::If));
}

optional_unique_ptr<EqualityExpr> Parser::ParseEqualityExpr() {
  return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "TODO 'assignexpr'", Token::If));
}

optional_unique_ptr<LogicalAndExpr> Parser::ParseLogicalAndExpr() {
  return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "TODO 'assignexpr'", Token::If));
}

optional_unique_ptr<LogicalOrExpr> Parser::ParseLogicalOrExpr() {
  return unexpected(ParseError(ParseError::Kind::MissingToken,
                                   "TODO 'assignexpr'", Token::If));
}

}