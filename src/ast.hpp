#ifndef AST_HPP
#define AST_HPP

/*
    This is a gigantic file!
*/

// This is predominantly based upon the EBNF
// of course changes will occur
// i.e. ArgumentExpressionList => Vector(Expression)

// @TODO: I'm a bit lazy rn so I've used template<typename T>
// in place of copy + pasting the constructor multiple times while in effect
// it shouldn't impact performance/codesize it will impact compile times
// (though I would presume a relatively negligible amount)
// regardless should be replaced as it isn't very explicit

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <optional>
#include <string>
#include <variant>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "printer_helpers.hpp"

class BaseAST {
protected:
    BaseAST(LineRange pos) : pos(pos) {}
public:
    LineRange pos;

    BaseAST() = delete;

    virtual json GetMetaData() const = 0;
};

#pragma region AST_PRE_DECLARATIONS

class FileLevelExpression;
class AssignmentExpression;
class TopLevelExpression;
class PrimaryExpression;
class FuncCall;
class PostfixExpression;
class UnaryExpression;
class PowerExpression;
class MultiplicativeExpression;
class AdditiveExpression;
class RelationalExpression;
class EqualityExpression;
class LogicalAndExpression;
class LogicalOrExpression;
class ConditionalExpression;
class Block;
class Expression;
class CompoundConditional;
class WhileLoop;
class ForLoop;
class ForLoopContents;
class ElseBlock;
class IfBlock;
class TupleMember;
class TupleDefinition;
class TypeExpression;
class TypeMember;
class FuncDefinition;
class Constant;
class ArrayConstant;
class MapConstant;

#pragma endregion

enum class AssignmentOp {
    AdditionEqual,
    SubtractionEqual,
    Equal,
    DivisionEqual,
    PowerEqual,
    ModulusEqual,
    MultiplyEqual,
    IntDivisionEqual,
};

const char *AssignmentOpToStr(AssignmentOp op);

enum class PostfixOp {
    Increment,
    Decrement,
};

const char *PostfixOpToStr(PostfixOp op);

enum class PrefixOp {
    Negate,
    Negative,
    Positive,
    Increment,
    Decrement,
};

const char *PrefixOpToStr(PrefixOp op);

enum class MultiplicativeOp {
    Multiplication,
    Division,
    Modulus,
    IntDivision,
};

const char *MultiplicativeOpToStr(MultiplicativeOp op);

enum class AdditiveOp {
    Subtraction,
    Addition,
};

const char *AdditiveOpToStr(AdditiveOp op);

enum class RelationalOp {
    GreaterThan,
    LessThan,
    GreaterThanEqual,
    LessThanEqual,
};

const char *RelationalOpToStr(RelationalOp op);

enum class EqualityOp {
    Equal,
    NotEqual,
};

const char *EqualityOpToStr(EqualityOp op);

class FileLevelExpression: public BaseAST {
public:
    std::vector<std::unique_ptr<TopLevelExpression>> expressions;

    FileLevelExpression(LineRange pos, std::vector<std::unique_ptr<TopLevelExpression>> exprs):
        expressions(std::move(exprs)), BaseAST(pos) {}

    json GetMetaData() const;
};

class TopLevelExpression: public BaseAST {
using ExprType = std::variant<std::unique_ptr<AssignmentExpression>, std::unique_ptr<Expression>>;

public:
    ExprType expr;
    bool ret; // 'return' expression

    TopLevelExpression(LineRange pos, ExprType expr, bool ret): BaseAST(pos), ret(ret), expr(std::move(expr)) {}

    json GetMetaData() const;
};

class PrimaryExpression: public BaseAST {
using ExprType = std::variant<std::string, std::unique_ptr<Expression>, std::unique_ptr<Constant>, std::unique_ptr<TypeExpression>>;

public:
    ExprType expr;

    template<typename T>
    PrimaryExpression(LineRange pos, T arg): BaseAST(pos), expr(std::move(arg)) {}

    json GetMetaData() const;
};

class AssignmentExpression: public BaseAST {
using DeclareVariable = std::tuple<std::unique_ptr<TupleDefinition>, std::optional<std::unique_ptr<Expression>>>;
using StandardAssignment = std::tuple<std::unique_ptr<Expression>, AssignmentOp, std::unique_ptr<Expression>>;

public:
    std::variant<DeclareVariable, StandardAssignment> expr;

    AssignmentExpression(LineRange pos, std::unique_ptr<TupleDefinition> def, std::optional<std::unique_ptr<Expression>> expr):
        expr(std::make_tuple(std::move(def), std::move(expr))), BaseAST(pos) {}

    AssignmentExpression(LineRange pos, std::unique_ptr<Expression> lhs, AssignmentOp op, std::unique_ptr<Expression> rhs):
        expr(std::make_tuple(std::move(lhs), op, std::move(rhs))), BaseAST(pos) {}

    json GetMetaData() const;
};

class FuncCall: public BaseAST {
public:
    std::unique_ptr<PostfixExpression> func;
    std::vector<std::unique_ptr<Expression>> args;

    FuncCall(LineRange pos, std::unique_ptr<PostfixExpression> expr, std::vector<std::unique_ptr<Expression>> args):
        BaseAST(pos), func(std::move(expr)), args(std::move(args)) {}

    json GetMetaData() const;
};

class PostfixExpression: public BaseAST {
using IndexExpr = std::tuple<std::unique_ptr<PostfixExpression>, std::unique_ptr<Expression>>;
using SliceExpr = std::tuple<std::unique_ptr<PostfixExpression>,
                             std::optional<std::unique_ptr<Expression>>,
                             std::optional<std::unique_ptr<Expression>>,
                             std::optional<std::unique_ptr<Expression>>>;
using FoldExpr = std::tuple<std::unique_ptr<FuncCall>, std::unique_ptr<PostfixExpression>>;
using PostfixOpExpr = std::tuple<std::unique_ptr<PostfixExpression>, PostfixOp>;
using MemberAccessExpr = std::tuple<std::unique_ptr<PostfixExpression>, std::string>;
using MacroExpr = std::tuple<std::string, std::vector<std::string>>;

public:
    std::variant<std::unique_ptr<PrimaryExpression>, IndexExpr, SliceExpr, std::unique_ptr<FuncCall>, FoldExpr, PostfixOpExpr, MemberAccessExpr, MacroExpr> expr;

    PostfixExpression(LineRange pos, std::unique_ptr<PrimaryExpression> expr): BaseAST(pos), expr(std::move(expr)) {}

    PostfixExpression(LineRange pos, std::unique_ptr<PostfixExpression> lhs, std::unique_ptr<Expression> index): BaseAST(pos), expr(std::make_tuple(std::move(lhs), std::move(index))) {}

    PostfixExpression(LineRange pos, std::unique_ptr<PostfixExpression> lhs, std::optional<std::unique_ptr<Expression>> start, std::optional<std::unique_ptr<Expression>> stop, std::optional<std::unique_ptr<Expression>> step):
        BaseAST(pos), expr(std::make_tuple(std::move(lhs), std::move(start), std::move(stop), std::move(step))) {}

    PostfixExpression(LineRange pos, std::unique_ptr<FuncCall> func_call): BaseAST(pos), expr(std::move(func_call)) {}

    PostfixExpression(LineRange pos, std::unique_ptr<FuncCall> func_call, std::unique_ptr<PostfixExpression> postfix):
        BaseAST(pos), expr(std::make_tuple(std::move(func_call), std::move(postfix))) {}

    PostfixExpression(LineRange pos, std::unique_ptr<PostfixExpression> postfix, PostfixOp op):
        BaseAST(pos), expr(std::make_tuple(std::move(postfix), op)) {}

    PostfixExpression(LineRange pos, std::unique_ptr<PostfixExpression> postfix, std::string to_access):
        BaseAST(pos), expr(std::make_tuple(std::move(postfix), to_access)) {}

    PostfixExpression(LineRange pos, std::string str, std::vector<std::string> sub_strs):
        BaseAST(pos), expr(std::make_tuple(str, sub_strs)) {}

    json GetMetaData() const;
};

class UnaryExpression: public BaseAST {
using PrefixOpExpr = std::tuple<std::unique_ptr<UnaryExpression>, PrefixOp>;

public:
    std::variant<std::unique_ptr<PostfixExpression>, PrefixOpExpr> expr;

    UnaryExpression(LineRange pos, std::unique_ptr<PostfixExpression> expr): expr(std::move(expr)), BaseAST(pos) {}
    UnaryExpression(LineRange pos, std::unique_ptr<UnaryExpression> unary, PrefixOp op): expr(std::make_tuple(std::move(unary), op)), BaseAST(pos) {}

    json GetMetaData() const;
};

class PowerExpression: public BaseAST {
using PowerOpExpr = std::tuple<std::unique_ptr<PowerExpression>, std::unique_ptr<UnaryExpression>>;

public:
    std::variant<std::unique_ptr<UnaryExpression>, PowerOpExpr> expr;

    PowerExpression(LineRange pos, std::unique_ptr<UnaryExpression> expr): expr(std::move(expr)), BaseAST(pos) {}
    PowerExpression(LineRange pos, std::unique_ptr<PowerExpression> lhs, std::unique_ptr<UnaryExpression> rhs): expr(std::make_tuple(std::move(lhs), std::move(rhs))), BaseAST(pos) {}

    json GetMetaData() const;
};

class MultiplicativeExpression: public BaseAST {
using MulOpExpr = std::tuple<std::unique_ptr<MultiplicativeExpression>, MultiplicativeOp, std::unique_ptr<PowerExpression>>;

public:
    std::variant<std::unique_ptr<PowerExpression>, MulOpExpr> expr;

    MultiplicativeExpression(LineRange pos, std::unique_ptr<PowerExpression> expr): expr(std::move(expr)), BaseAST(pos) {}
    MultiplicativeExpression(LineRange pos, std::unique_ptr<MultiplicativeExpression> lhs, MultiplicativeOp op, std::unique_ptr<PowerExpression> rhs): expr(std::make_tuple(std::move(lhs), op, std::move(rhs))), BaseAST(pos) {}

    json GetMetaData() const;
};

class AdditiveExpression: public BaseAST {
using AddOpExpr = std::tuple<std::unique_ptr<AdditiveExpression>, AdditiveOp, std::unique_ptr<MultiplicativeExpression>>;

public:
    std::variant<std::unique_ptr<MultiplicativeExpression>, AddOpExpr> expr;

    AdditiveExpression(LineRange pos, std::unique_ptr<MultiplicativeExpression> expr): expr(std::move(expr)), BaseAST(pos) {}
    AdditiveExpression(LineRange pos, std::unique_ptr<AdditiveExpression> lhs, AdditiveOp op, std::unique_ptr<MultiplicativeExpression> rhs): expr(std::make_tuple(std::move(lhs), op, std::move(rhs))), BaseAST(pos) {}

    json GetMetaData() const;
};

class RelationalExpression: public BaseAST {
using RelationalOpExpr = std::tuple<std::unique_ptr<RelationalExpression>, RelationalOp, std::unique_ptr<AdditiveExpression>>;

public:
    std::variant<std::unique_ptr<AdditiveExpression>, RelationalOpExpr> expr;

    RelationalExpression(LineRange pos, std::unique_ptr<AdditiveExpression> expr): expr(std::move(expr)), BaseAST(pos) {}
    RelationalExpression(LineRange pos, std::unique_ptr<RelationalExpression> lhs, RelationalOp op, std::unique_ptr<AdditiveExpression> rhs): expr(std::make_tuple(std::move(lhs), op, std::move(rhs))), BaseAST(pos) {}

    json GetMetaData() const;
};

class EqualityExpression: public BaseAST {
using EqualityOpExpr = std::tuple<std::unique_ptr<EqualityExpression>, EqualityOp, std::unique_ptr<RelationalExpression>>;

public:
    std::variant<std::unique_ptr<RelationalExpression>, EqualityOpExpr> expr;

    EqualityExpression(LineRange pos, std::unique_ptr<RelationalExpression> expr): expr(std::move(expr)), BaseAST(pos) {}
    EqualityExpression(LineRange pos, std::unique_ptr<EqualityExpression> lhs, EqualityOp op, std::unique_ptr<RelationalExpression> rhs): expr(std::make_tuple(std::move(lhs), op, std::move(rhs))), BaseAST(pos) {}

    json GetMetaData() const;
};

class LogicalAndExpression: public BaseAST {
using LogicalAndExpr = std::tuple<std::unique_ptr<LogicalAndExpression>, std::unique_ptr<EqualityExpression>>;

public:
    std::variant<std::unique_ptr<EqualityExpression>, LogicalAndExpr> expr;

    LogicalAndExpression(LineRange pos, std::unique_ptr<EqualityExpression> expr): expr(std::move(expr)), BaseAST(pos) {}
    LogicalAndExpression(LineRange pos, std::unique_ptr<LogicalAndExpression> lhs, std::unique_ptr<EqualityExpression> rhs): expr(std::make_tuple(std::move(lhs), std::move(rhs))), BaseAST(pos) {}

    json GetMetaData() const;
};

class LogicalOrExpression: public BaseAST {
using LogicalOrExpr = std::tuple<std::unique_ptr<LogicalOrExpression>, std::unique_ptr<LogicalAndExpression>>;

public:
    std::variant<std::unique_ptr<LogicalAndExpression>, LogicalOrExpr> expr;

    LogicalOrExpression(LineRange pos, std::unique_ptr<LogicalAndExpression> expr): expr(std::move(expr)), BaseAST(pos) {}
    LogicalOrExpression(LineRange pos, std::unique_ptr<LogicalOrExpression> lhs, std::unique_ptr<LogicalAndExpression> rhs): expr(std::make_tuple(std::move(lhs), std::move(rhs))), BaseAST(pos) {}

    json GetMetaData() const;
};

class ConditionalExpression: public BaseAST {
public:
    std::unique_ptr<LogicalOrExpression> cond;
    std::optional<std::unique_ptr<Expression>> ternary_true;
    std::optional<std::unique_ptr<Expression>> ternary_false;

    ConditionalExpression(LineRange pos, std::unique_ptr<LogicalOrExpression> expr): cond(std::move(expr)),
        ternary_true(std::nullopt), ternary_false(std::nullopt), BaseAST(pos) {}
    
    ConditionalExpression(LineRange pos, std::unique_ptr<LogicalOrExpression> expr,
        std::unique_ptr<Expression> if_true, std::unique_ptr<Expression> if_false): BaseAST(pos),
            cond(std::move(expr)), ternary_true(std::move(if_true)), ternary_false(std::move(if_false)) {}

    bool IsTernary() const { return this->ternary_true && this->ternary_false; }

    json GetMetaData() const;
};

class Block: public BaseAST {
public:
    std::vector<std::unique_ptr<TopLevelExpression>> exprs;

    Block(LineRange pos, std::vector<std::unique_ptr<TopLevelExpression>> exprs): BaseAST(pos), exprs(std::move(exprs)) {}

    json GetMetaData() const;
};

class Expression: public BaseAST {
using FuncDefBlock = std::tuple<std::unique_ptr<FuncDefinition>, std::unique_ptr<Block>>;
using TupleDefBlock = std::tuple<std::unique_ptr<TupleDefinition>, std::unique_ptr<Block>>;

public:
    std::variant<std::unique_ptr<ConditionalExpression>, FuncDefBlock, TupleDefBlock, std::unique_ptr<CompoundConditional>> expr;

    template<typename T>
    Expression(LineRange pos, std::unique_ptr<T> arg): expr(std::move(arg)), BaseAST(pos) {}

    template<typename T>
    Expression(LineRange pos, std::unique_ptr<T> arg, std::unique_ptr<Block> attached):
        expr(std::make_tuple(std::move(arg), std::move(attached))), BaseAST(pos) {}

    json GetMetaData() const;
};

class CompoundConditional: public BaseAST {
public:
    std::variant<std::unique_ptr<ForLoop>, std::unique_ptr<WhileLoop>, std::unique_ptr<IfBlock>> expr;

    template<typename T>
    CompoundConditional(LineRange pos, std::unique_ptr<T> arg): expr(std::move(arg)), BaseAST(pos) {}

    json GetMetaData() const;
};

class WhileLoop: public BaseAST {
public:
    std::unique_ptr<ConditionalExpression> expr;
    std::unique_ptr<Block> block;
    std::optional<std::unique_ptr<Block>> else_block;

    WhileLoop(LineRange pos, std::unique_ptr<ConditionalExpression> expr, std::unique_ptr<Block> block):
        BaseAST(pos), expr(std::move(expr)), block(std::move(block)), else_block(std::nullopt) {}

    WhileLoop(LineRange pos, std::unique_ptr<ConditionalExpression> expr, std::unique_ptr<Block> block, std::unique_ptr<Block> else_block):
        BaseAST(pos), expr(std::move(expr)), block(std::move(block)), else_block(std::move(else_block)) {}

    json GetMetaData() const;
};

class ForLoopContents: public BaseAST {
using ForIn = std::tuple<std::vector<std::string>, std::unique_ptr<ConditionalExpression>>;
using ForTraditional = std::tuple<
    std::optional<std::unique_ptr<AssignmentExpression>>,
    std::optional<std::unique_ptr<ConditionalExpression>>,
    std::optional<std::unique_ptr<ConditionalExpression>>>;

public:
    std::variant<ForIn, ForTraditional> expr;

    ForLoopContents(LineRange pos, std::vector<std::string> ids, std::unique_ptr<ConditionalExpression> expr):
        expr(std::make_tuple(ids, std::move(expr))), BaseAST(pos) {}

    ForLoopContents(LineRange pos, std::optional<std::unique_ptr<AssignmentExpression>> start,
        std::optional<std::unique_ptr<ConditionalExpression>> stop, std::optional<std::unique_ptr<ConditionalExpression>> step):
            expr(std::make_tuple(std::move(start), std::move(stop), std::move(step))), BaseAST(pos) {}

    json GetMetaData() const;
};

class ForLoop: public BaseAST {
public:
    ForLoopContents expr;
    std::unique_ptr<Block> block;
    std::optional<std::unique_ptr<Block>> else_block;

    ForLoop(LineRange pos, ForLoopContents expr, std::unique_ptr<Block> block):
        BaseAST(pos), expr(std::move(expr)), block(std::move(block)), else_block(std::nullopt) {}

    ForLoop(LineRange pos, ForLoopContents expr, std::unique_ptr<Block> block, std::unique_ptr<Block> else_block):
        BaseAST(pos), expr(std::move(expr)), block(std::move(block)), else_block(std::move(else_block)) {}

    json GetMetaData() const;
};

class IfBlock: public BaseAST {
public:
    std::unique_ptr<ConditionalExpression> cond;
    std::unique_ptr<Block> block;
    std::optional<std::unique_ptr<ElseBlock>> else_block;

    IfBlock(LineRange pos, std::unique_ptr<ConditionalExpression> cond, std::unique_ptr<Block> block): BaseAST(pos),
        cond(std::move(cond)), block(std::move(block)) {}

    IfBlock(LineRange pos, std::unique_ptr<ConditionalExpression> cond, std::unique_ptr<Block> block, std::unique_ptr<ElseBlock> else_block): BaseAST(pos),
        cond(std::move(cond)), block(std::move(block)), else_block(std::move(else_block)) {}

    json GetMetaData() const;
};

class ElseBlock: public BaseAST {
public:
    std::variant<std::unique_ptr<IfBlock>, std::unique_ptr<Block>> expr;

    template<typename T>
    ElseBlock(LineRange pos, std::unique_ptr<T> arg): expr(std::move(arg)), BaseAST(pos) {}

    json GetMetaData() const;
};

class TupleMember: public BaseAST {
public:
    std::string id;
    std::optional<std::unique_ptr<TypeExpression>> type;

    TupleMember(LineRange pos, std::string id): id(id), type(std::nullopt), BaseAST(pos) {}
    TupleMember(LineRange pos, std::string id, std::unique_ptr<TypeExpression> expr): id(id), type(std::move(expr)), BaseAST(pos) {}

    json GetMetaData() const;
};

class TupleDefinition: public BaseAST {
public:
    std::vector<std::unique_ptr<TupleMember>> members;

    TupleDefinition(LineRange pos, std::vector<std::unique_ptr<TupleMember>> members): BaseAST(pos), members(std::move(members)) {}

    json GetMetaData() const;
};

class TypeMember: public BaseAST {
public:
    std::variant<std::unique_ptr<FuncCall>, std::string, std::unique_ptr<FuncDefinition>, std::unique_ptr<TupleDefinition>,
        std::tuple<std::unique_ptr<TypeExpression>, std::optional<std::unique_ptr<TypeExpression>>>> expr;

    template<typename T>
    TypeMember(LineRange pos, std::unique_ptr<T> arg): expr(std::move(arg)), BaseAST(pos) {}

    TypeMember(LineRange pos, std::unique_ptr<TypeExpression> lhs, std::optional<std::unique_ptr<TypeExpression>> rhs):
        BaseAST(pos), expr(std::make_tuple(std::move(lhs), std::move(rhs))) {}

    json GetMetaData() const;
};

class TypeExpression: public BaseAST {
public:
    std::variant<std::unique_ptr<TypeMember>, std::tuple<std::unique_ptr<TypeExpression>, std::optional<std::unique_ptr<TypeMember>>, std::optional<std::unique_ptr<TypeMember>>>> expr;

    TypeExpression(LineRange pos, std::unique_ptr<TypeMember> expr): BaseAST(pos), expr(std::move(expr)) {}
    TypeExpression(LineRange pos, std::unique_ptr<TypeExpression> lhs, std::optional<std::unique_ptr<TypeMember>> or_expr, std::optional<std::unique_ptr<TypeMember>> impl_expr):
        BaseAST(pos), expr(std::make_tuple(std::move(lhs), std::move(or_expr), std::move(impl_expr))) {}

    json GetMetaData() const;
};

class FuncDefinition: public BaseAST {
public:
    std::optional<std::unique_ptr<TupleDefinition>> args;
    std::optional<std::unique_ptr<TypeExpression>> ret_type;

    FuncDefinition(LineRange pos): BaseAST(pos), args(std::nullopt), ret_type(std::nullopt) {}
    FuncDefinition(LineRange pos, std::unique_ptr<TupleDefinition> args): BaseAST(pos), args(std::move(args)), ret_type(std::nullopt) {}
    FuncDefinition(LineRange pos, std::unique_ptr<TupleDefinition> args, std::unique_ptr<TypeExpression> ret_type): BaseAST(pos), args(std::move(args)), ret_type(std::move(ret_type)) {}

    json GetMetaData() const;
};

class Constant: public BaseAST {
public:
    std::variant<std::unique_ptr<ArrayConstant>, std::unique_ptr<MapConstant>, double, long int, std::string, char> data;

    template<typename T>
    Constant(LineRange pos, T arg): data(arg), BaseAST(pos) {}

    json GetMetaData() const;
};

class MapConstant: public BaseAST {
public:
    std::vector<std::unique_ptr<Expression>> keys;
    std::vector<std::unique_ptr<Expression>> values;

    MapConstant(LineRange pos, std::vector<std::unique_ptr<Expression>> keys, std::vector<std::unique_ptr<Expression>> values):
        BaseAST(pos), keys(std::move(keys)), values(std::move(values)) {}

    json GetMetaData() const;
};

class ArrayConstant: public BaseAST {
public:
    std::vector<std::unique_ptr<Expression>> values;

    ArrayConstant(LineRange pos, std::vector<std::unique_ptr<Expression>> values):
        BaseAST(pos), values(std::move(values)) {}

    json GetMetaData() const;
};

#endif