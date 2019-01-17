#include "parser.hpp"

namespace porc::internals {

std::unique_ptr<FileLevelExpr> Parser::ParseFileLevelExp() {}
std::unique_ptr<TopLevelExpr> Parser::ParseTopLevelExpr() {}
std::unique_ptr<PrimaryExpr> Parser::ParserPrimaryExpr() {}
std::unique_ptr<AssignmentExpr> Parser::ParserAssignmentExpr() {}
std::unique_ptr<FuncCall> Parser::ParserFuncCall() {}
std::unique_ptr<PostfixExpr> Parser::ParserPostfixExpr() {}
std::unique_ptr<UnaryExpr> Parser::ParserUnaryExpr() {}
std::unique_ptr<PowerExpr> Parser::ParserPrefixExpr() {}
std::unique_ptr<MultiplicativeExpr> Parser::ParseMultiplicativeExpr() {}
std::unique_ptr<AdditiveExpr> Parser::ParseAdditiveExpr() {}
std::unique_ptr<RelationalExpr> Parser::ParseRelationalExpr() {}
std::unique_ptr<EqualityExpr> Parser::ParseEqualityExpr() {}
std::unique_ptr<LogicalAndExpr> Parser::ParseLogicalAndExpr() {}
std::unique_ptr<LogicalOrExpr> Parser::ParseLogicalOrExpr() {}
std::unique_ptr<ConditionalExpr> Parser::ParseConditionalExpr() {}
std::unique_ptr<Block> Parser::ParseBlockExpr() {}
std::unique_ptr<Expr> Parser::ParseExpr() {}
std::unique_ptr<WhileLoop> Parser::ParseWhileLoop() {}
std::unique_ptr<ForLoop> Parser::ParseForLoop() {}
std::unique_ptr<ForLoopContents> Parser::ParseForLoopContents() {}
std::unique_ptr<ElseBlock> Parser::ParseElseBlock() {}
std::unique_ptr<IfBlock> Parser::ParseIfBlock() {}
std::unique_ptr<TupleMember> Parser::ParseTupleMember() {}
std::unique_ptr<TupleDefinition> Parser::ParseTupleDef() {}
std::unique_ptr<TypeExpr> Parser::ParseTypeExpr() {}
std::unique_ptr<TypeMember> Parser::ParseTypeMember() {}
std::unique_ptr<FuncDefinition> Parser::ParseFuncDef() {}
std::unique_ptr<Constant> Parser::ParseConstant() {}
std::unique_ptr<ArrayConstant> Parser::ParseArrayConstant() {}
std::unique_ptr<MapConstant> Parser::ParseMapConstant() {}

}