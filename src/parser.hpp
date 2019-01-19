#ifndef PARSER_HPP
#define PARSER_HPP

#include "token_stream.hpp"
#include "ast.hpp"

#include <expected.hpp>
#include <string>

namespace porc::internals {

template<typename T, typename E>
using expected_unique_ptr = tl::expected<std::unique_ptr<T>, E>;

template<typename T>
using optional_unique_ptr = std::optional<std::unique_ptr<T>>;

struct ParseError {
  enum class Kind {
    MissingToken, // missing a token (not EOF)
    InvalidToken, // wasn't expecting this token
    ValidEOF,     // not really an error more just info that we ran out
    InvalidEOF,   // wasn't expecting EOF
  };

public:
  Kind kind;
  std::string extra_info;
  Token related_token;

  ParseError(Kind kind) : kind(kind) {}
  ParseError(Kind kind, std::string extra_info)
      : kind(kind), extra_info(extra_info) {}
  ParseError(Kind kind, std::string extra_info, Token related_token)
      : kind(kind), extra_info(extra_info), related_token(related_token) {}
};

class Parser {
  template<typename T>
  using expected_expr = expected_unique_ptr<T, ParseError>;


private:
  TokenStream stream;

public:
  Parser(TokenStream stream) : stream(std::move(stream)) {}

  void UnexpectedToken(Token actual, Token expected);
  void UnexpectedEndOfFile(TokenType expected);
  bool ConsumeTokenType(TokenType type);

  expected_expr<FileLevelExpr> ParseFileLevelExpr();
  expected_expr<TopLevelExpr> ParseTopLevelExpr();
  expected_expr<PrimaryExpr> ParsePrimaryExpr();
  expected_expr<AssignmentExpr> ParseAssignmentExpr();
  expected_expr<FuncCall> ParseFuncCall();
  expected_expr<PostfixExpr> ParsePostfixExpr();
  expected_expr<UnaryExpr> ParseUnaryExpr();
  expected_expr<PowerExpr> ParsePrefixExpr();
  expected_expr<MultiplicativeExpr> ParseMultiplicativeExpr();
  expected_expr<AdditiveExpr> ParseAdditiveExpr();
  expected_expr<RelationalExpr> ParseelationalExpr();
  expected_expr<EqualityExpr> ParseEqualityExpr();
  expected_expr<LogicalAndExpr> ParseLogicalAndExpr();
  expected_expr<LogicalOrExpr> ParseLogicalOrExpr();
  expected_expr<ConditionalExpr> ParseConditionalExpr();
  expected_expr<Block> ParseBlockExpr();
  expected_expr<Expr> ParseExpr();
  expected_expr<WhileLoop> ParseWhileLoop();
  expected_expr<ForLoop> ParseForLoop();
  expected_expr<ForLoopContents> ParseForLoopContents();
  expected_expr<ElseBlock> ParseElseBlock();
  expected_expr<IfBlock> ParseIfBlock();
  expected_expr<TupleMember> ParseTupleMember();
  expected_expr<TupleDefinition> ParseTupleDef();
  expected_expr<TypeExpr> ParseTypeExpr();
  expected_expr<TypeMember> ParseTypeMember();
  expected_expr<FuncDefinition> ParseFuncDef();
  expected_expr<Constant> ParseConstant();
  expected_expr<ArrayConstant> ParseArrayConstant();
  expected_expr<MapConstant> ParseMapConstant();
};

}

#endif