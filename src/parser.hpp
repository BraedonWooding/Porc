#ifndef PARSER_HPP
#define PARSER_HPP

#include "token_stream.hpp"
#include "ast.hpp"

class Parser {
private:
  TokenStream stream;

public:
  Parser(TokenStream stream): stream(std::move(stream)) {}

  std::unique_ptr<FileLevelExpr> ParseFileLevelExp();
  std::unique_ptr<TopLevelExpr> ParseTopLevelExpr();
  std::unique_ptr<PrimaryExpr> ParserPrimaryExpr();
  std::unique_ptr<AssignmentExpr> ParserAssignmentExpr();
  std::unique_ptr<FuncCall> ParserFuncCall();
  std::unique_ptr<PostfixExpr> ParserPostfixExpr();
  std::unique_ptr<UnaryExpr> ParserUnaryExpr();
  std::unique_ptr<PowerExpr> ParserPrefixExpr();
  std::unique_ptr<MultiplicativeExpr> ParseMultiplicativeExpr();
  std::unique_ptr<AdditiveExpr> ParseAdditiveExpr();
  std::unique_ptr<RelationalExpr> ParseRelationalExpr();
  std::unique_ptr<EqualityExpr> ParseEqualityExpr();
  std::unique_ptr<LogicalAndExpr> ParseLogicalAndExpr();
  std::unique_ptr<LogicalOrExpr> ParseLogicalOrExpr();
  std::unique_ptr<ConditionalExpr> ParseConditionalExpr();
  std::unique_ptr<Block> ParseBlockExpr();
  std::unique_ptr<Expr> ParseExpr();
  std::unique_ptr<WhileLoop> ParseWhileLoop();
  std::unique_ptr<ForLoop> ParseForLoop();
  std::unique_ptr<ForLoopContents> ParseForLoopContents();
  std::unique_ptr<ElseBlock> ParseElseBlock();
  std::unique_ptr<IfBlock> ParseIfBlock();
  std::unique_ptr<TupleMember> ParseTupleMember();
  std::unique_ptr<TupleDefinition> ParseTupleDef();
  std::unique_ptr<TypeExpr> ParseTypeExpr();
  std::unique_ptr<TypeMember> ParseTypeMember();
  std::unique_ptr<FuncDefinition> ParseFuncDef();
  std::unique_ptr<Constant> ParseConstant();
  std::unique_ptr<ArrayConstant> ParseArrayConstant();
  std::unique_ptr<MapConstant> ParseMapConstant();
};

#endif