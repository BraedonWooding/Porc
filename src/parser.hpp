#ifndef PARSER_HPP
#define PARSER_HPP

#include "tokenizer.hpp"
#include "ast.hpp"

class Parser {
private:
    Tokenizer tok;

public:
    Parser(Tokenizer tok): tok(tok) {}

    std::unique_ptr<FileLevelExpression> ParseFileLevelExp();
    std::unique_ptr<TopLevelExpression> ParseTopLevelExpr();
    std::unique_ptr<PrimaryExpression> ParserPrimaryExpr();
    std::unique_ptr<AssignmentExpression> ParserAssignmentExpr();
    std::unique_ptr<FuncCall> ParserFuncCall();
    std::unique_ptr<PostfixExpression> ParserPostfixExpr();
    std::unique_ptr<UnaryExpression> ParserUnaryExpr();
    std::unique_ptr<PowerExpression> ParserPrefixExpr();
    std::unique_ptr<MultiplicativeExpression> ParseMultiplicativeExpr();
    std::unique_ptr<AdditiveExpression> ParseAdditiveExpr();
    std::unique_ptr<RelationalExpression> ParseRelationalExpr();
    std::unique_ptr<EqualityExpression> ParseEqualityExpr();
    std::unique_ptr<LogicalAndExpression> ParseLogicalAndExpr();
    std::unique_ptr<LogicalOrExpression> ParseLogicalOrExpr();
    std::unique_ptr<ConditionalExpression> ParseConditionalExpr();
    std::unique_ptr<Block> ParseBlockExpr();
    std::unique_ptr<Expression> ParseExpr();
    std::unique_ptr<CompoundConditional> ParseCompoundConditional();
    std::unique_ptr<WhileLoop> ParseWhileLoop();
    std::unique_ptr<ForLoop> ParseForLoop();
    std::unique_ptr<ForLoopContents> ParseForLoopContents();
    std::unique_ptr<ElseBlock> ParseElseBlock();
    std::unique_ptr<IfBlock> ParseIfBlock();
    std::unique_ptr<TupleMember> ParseTupleMember();
    std::unique_ptr<TupleDefinition> ParseTupleDef();
    std::unique_ptr<TypeExpression> ParseTypeExpr();
    std::unique_ptr<TypeMember> ParseTypeMember();
    std::unique_ptr<FuncDefinition> ParseFuncDef();
    std::unique_ptr<Constant> ParseConstant();
    std::unique_ptr<ArrayConstant> ParseArrayConstant();
    std::unique_ptr<MapConstant> ParseMapConstant();
};

#endif