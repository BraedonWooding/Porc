#include "parser.hpp"

#include <rang.hpp>

namespace porc::internals {

template<typename T>
using expected_expr = Parser::expected_expr<T>;

void Parser::UnexpectedEndOfFile(TokenType expected) {
  std::cerr << rang::style::reset << rang::style::bold << rang::fg::red
            << "Unexpected EOF was expecting "
            << Token(expected, LineRange::NullRange()).ToErrorMsg() << "\n";
}

void Parser::UnexpectedToken(Token actual, Token expected) {
  std::cerr << rang::style::reset << rang::style::bold << rang::fg::red
            << "Unexpected Token " << actual.pos << " " << actual.ToErrorMsg()
            << " was expecting " << expected.ToErrorMsg() << "\n";
}

bool Parser::ConsumeTokenType(TokenType type) {
  Token tok = stream.Next();
  if (tok.type == TokenType::EndOfFile) {
    UnexpectedEndOfFile(type);
    return false;
  } else if (tok.type != type) {
    UnexpectedToken(tok, Token(type, LineRange::NullRange()));
    return false;
  }

  return true;
}

template<typename T>
LineRange FindRangeOfVector(std::vector<std::unique_ptr<T>> &vec) {
  if (vec.size() == 0) return LineRange::NullRange();
  else if (vec.size() == 1) return vec.at(0)->pos;
  else return LineRange(vec.at(0)->pos, vec.at(vec.size() - 1)->pos);
}

expected_expr<FileLevelExpr> Parser::ParseFileLevelExpr() {
  std::vector<std::unique_ptr<TopLevelExpr>> expressions;
  expected_expr<TopLevelExpr> expr;
  while ((expr = ParseTopLevelExpr()).has_value()) {
    expressions.push_back(std::move(*expr));
  }
  if (!expr) return tl::unexpected(expr.error());
  LineRange range = FindRangeOfVector(expressions);
  return std::make_unique<FileLevelExpr>(range, std::move(expressions));
}

expected_expr<TopLevelExpr> Parser::ParseTopLevelExpr() {
  Token tok = stream.Next();
  if (tok.type == TokenType::EndOfFile)
    return tl::unexpected(ParseError(ParseError::Kind::ValidEOF));

  if (tok.type == TokenType::Return) {
    Token old = tok;
    tok = stream.Next();

    if (tok.type == TokenType::EndOfFile)
      return tl::unexpected(ParseError(ParseError::Kind::InvalidEOF,
        "`return` with no following expression", old));

    // has to be assignment
    auto expr = ParseExpr();
    if (expr) {
      auto top_level = std::move(*expr);
      auto pos = top_level->pos;
      return std::make_unique<TopLevelExpr>(pos, std::move(top_level), true);
    }
    if (!expr) return tl::unexpected(expr.error());
  } else {
    if (tok.type == TokenType::Identifier) {
      // possibly assignment
      stream.Push(tok);
      auto tuple_def = ParseTupleDef();
      if (tuple_def) {
        // assignment
        
      } else {
        // expression
        auto expr = ParseExpr();
        if (expr) {
          auto top_level = std::move(*expr);
          auto pos = top_level->pos;
          return std::make_unique<TopLevelExpr>(pos, std::move(top_level), true);
        }
        if (!expr) return tl::unexpected(expr.error());
      }
    } else {
      // can't be assignment unless followed by assignment op
      auto expr = ParseExpr();
      if (!expr) return tl::unexpected(expr.error());
    }
  }

  return tl::unexpected(ParseError(ParseError::Kind::MissingToken,
    "Was expecting an expression couldn't form it from token", tok));
}

expected_expr<PrimaryExpr> Parser::ParsePrimaryExpr() {

}

expected_expr<AssignmentExpr> Parser::ParseAssignmentExpr() {

}

expected_expr<FuncCall> Parser::ParseFuncCall() {

}

expected_expr<PostfixExpr> Parser::ParsePostfixExpr() {

}

expected_expr<UnaryExpr> Parser::ParseUnaryExpr() {}
expected_expr<PowerExpr> Parser::ParsePrefixExpr() {}
expected_expr<MultiplicativeExpr> Parser::ParseMultiplicativeExpr() {}
expected_expr<AdditiveExpr> Parser::ParseAdditiveExpr() {}
expected_expr<RelationalExpr> Parser::ParseelationalExpr() {}
expected_expr<EqualityExpr> Parser::ParseEqualityExpr() {}
expected_expr<LogicalAndExpr> Parser::ParseLogicalAndExpr() {}
expected_expr<LogicalOrExpr> Parser::ParseLogicalOrExpr() {}
expected_expr<ConditionalExpr> Parser::ParseConditionalExpr() {}
expected_expr<Block> Parser::ParseBlockExpr() {}
expected_expr<Expr> Parser::ParseExpr() {}
expected_expr<WhileLoop> Parser::ParseWhileLoop() {}
expected_expr<ForLoop> Parser::ParseForLoop() {}
expected_expr<ForLoopContents> Parser::ParseForLoopContents() {}
expected_expr<ElseBlock> Parser::ParseElseBlock() {}
expected_expr<IfBlock> Parser::ParseIfBlock() {}
expected_expr<TupleMember> Parser::ParseTupleMember() {}
expected_expr<TupleDefinition> Parser::ParseTupleDef() {}
expected_expr<TypeExpr> Parser::ParseTypeExpr() {}
expected_expr<TypeMember> Parser::ParseTypeMember() {}
expected_expr<FuncDefinition> Parser::ParseFuncDef() {}
expected_expr<Constant> Parser::ParseConstant() {}
expected_expr<ArrayConstant> Parser::ParseArrayConstant() {}
expected_expr<MapConstant> Parser::ParseMapConstant() {}

}