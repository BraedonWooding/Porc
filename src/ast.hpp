#ifndef AST_HPP
#define AST_HPP

// This is predominantly based upon the EBNF
// of course changes will occur
// i.e. ArgumentExprList => Vector(Expr)

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

class FileLevelExpr;
class AssignmentExpr;
class TopLevelExpr;
class PrimaryExpr;
class FuncCall;
class PostfixExpr;
class UnaryExpr;
class PowerExpr;
class MultiplicativeExpr;
class AdditiveExpr;
class RelationalExpr;
class EqualityExpr;
class LogicalAndExpr;
class LogicalOrExpr;
class ConditionalExpr;
class Block;
class Expr;
class WhileLoop;
class ForLoop;
class ForLoopContents;
class ElseBlock;
class IfBlock;
class TupleMember;
class TupleDefinition;
class TypeExpr;
class TypeMember;
class FuncDefinition;
class Constant;
class ArrayConstant;
class MapConstant;

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

class FileLevelExpr : public BaseAST {
 public:
  std::vector<std::unique_ptr<TopLevelExpr>> Exprs;

  FileLevelExpr(LineRange pos, std::vector<std::unique_ptr<TopLevelExpr>> exprs)
      : Exprs(std::move(exprs)), BaseAST(pos) {}

  json GetMetaData() const;
};

class TopLevelExpr : public BaseAST {
using ExprType = std::variant<std::unique_ptr<AssignmentExpr>,
                              std::unique_ptr<Expr>>;

 public:
  ExprType expr;
  bool ret; // 'return' Expr

  TopLevelExpr(LineRange pos, ExprType expr, bool ret)
      : BaseAST(pos), ret(ret), expr(std::move(expr)) {}

  json GetMetaData() const;
};

class PrimaryExpr : public BaseAST {
using ExprType = std::variant<std::string, std::unique_ptr<Expr>,
                              std::unique_ptr<Constant>,
                              std::unique_ptr<TypeExpr>>;

 public:
  ExprType expr;

  PrimaryExpr(LineRange pos, std::string arg) : BaseAST(pos), expr(arg) {}
  PrimaryExpr(LineRange pos, std::unique_ptr<Expr> arg)
      : BaseAST(pos), expr(std::move(arg)) {}
  PrimaryExpr(LineRange pos, std::unique_ptr<Constant> arg)
      : BaseAST(pos), expr(std::move(arg)) {}
  PrimaryExpr(LineRange pos, std::unique_ptr<TypeExpr> arg)
      : BaseAST(pos), expr(std::move(arg)) {}

  json GetMetaData() const;
};

class AssignmentExpr : public BaseAST {
  struct Declare {
    std::unique_ptr<TupleDefinition> lhs;
    std::optional<std::unique_ptr<Expr>> rhs;
    Declare(std::unique_ptr<TupleDefinition> lhs,
            std::optional<std::unique_ptr<Expr>> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  };

  struct Assign {
    std::unique_ptr<Expr> lhs;
    AssignmentOp op;
    std::unique_ptr<Expr> rhs;
    Assign(std::unique_ptr<Expr> lhs, AssignmentOp op,
           std::unique_ptr<Expr> rhs)
        : lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}
  };

 public:
  std::variant<Declare, Assign> expr;

  AssignmentExpr(LineRange pos, std::unique_ptr<TupleDefinition> def,
                 std::optional<std::unique_ptr<Expr>> expr)
      : expr(Declare(std::move(def), std::move(expr))), BaseAST(pos) {}

  AssignmentExpr(LineRange pos, std::unique_ptr<Expr> lhs,
                 AssignmentOp op, std::unique_ptr<Expr> rhs)
      : expr(Assign(std::move(lhs), op, std::move(rhs))),
        BaseAST(pos) {}

  json GetMetaData() const;
};

class FuncCall : public BaseAST {
 public:
  std::unique_ptr<PostfixExpr> func;
  std::vector<std::unique_ptr<Expr>> args;

  FuncCall(LineRange pos, std::unique_ptr<PostfixExpr> expr,
           std::vector<std::unique_ptr<Expr>> args)
      : BaseAST(pos), func(std::move(expr)), args(std::move(args)) {}

  json GetMetaData() const;
};

class PostfixExpr : public BaseAST {
  struct IndexExpr {
    std::unique_ptr<PostfixExpr> lhs;
    std::unique_ptr<Expr> index;

    IndexExpr(std::unique_ptr<PostfixExpr> lhs,
              std::unique_ptr<Expr> index)
        : lhs(std::move(lhs)), index(std::move(index)) {}
  };

  struct SliceExpr {
    std::unique_ptr<PostfixExpr> obj;
    std::optional<std::unique_ptr<Expr>> start;
    std::optional<std::unique_ptr<Expr>> stop;
    std::optional<std::unique_ptr<Expr>> step;

    SliceExpr(std::unique_ptr<PostfixExpr> obj,
              std::optional<std::unique_ptr<Expr>> start,
              std::optional<std::unique_ptr<Expr>> stop,
              std::optional<std::unique_ptr<Expr>> step)
        : obj(std::move(obj)), start(std::move(start)), stop(std::move(stop)),
          step(std::move(step)) {}
  };

  struct FoldExpr {
    std::unique_ptr<FuncCall> func;
    std::unique_ptr<PostfixExpr> fold_expr;

    FoldExpr(std::unique_ptr<FuncCall> func,
             std::unique_ptr<PostfixExpr> fold_expr)
        : func(std::move(func)), fold_expr(std::move(fold_expr)) {}
  };

  struct PostfixOpExpr {
    std::unique_ptr<PostfixExpr> lhs;
    PostfixOp op;

    PostfixOpExpr(std::unique_ptr<PostfixExpr> lhs, PostfixOp op)
        : lhs(std::move(lhs)), op(op) {}
  };

  struct MemberAccessExpr {
    std::unique_ptr<PostfixExpr> lhs;
    std::string member;

    MemberAccessExpr(std::unique_ptr<PostfixExpr> lhs, std::string member)
        : lhs(std::move(lhs)), member(member) {}
  };

  struct MacroExpr {
    std::vector<std::string> qualifying_name;

    MacroExpr(std::vector<std::string> qualifying_name) 
        : qualifying_name(qualifying_name) {}
  };

 public:
  std::variant<std::unique_ptr<PrimaryExpr>, IndexExpr, SliceExpr,
               std::unique_ptr<FuncCall>, FoldExpr, PostfixOpExpr,
               MemberAccessExpr, MacroExpr> expr;

  PostfixExpr(LineRange pos, std::unique_ptr<PrimaryExpr> expr) 
      : BaseAST(pos), expr(std::move(expr)) {}

  PostfixExpr(LineRange pos, std::unique_ptr<PostfixExpr> lhs,
              std::unique_ptr<Expr> index)
      : BaseAST(pos), expr(IndexExpr(std::move(lhs), std::move(index))) {}

  PostfixExpr(LineRange pos, std::unique_ptr<PostfixExpr> lhs,
              std::optional<std::unique_ptr<Expr>> start,
              std::optional<std::unique_ptr<Expr>> stop,
              std::optional<std::unique_ptr<Expr>> step)
      : BaseAST(pos), expr(SliceExpr(std::move(lhs), std::move(start),
        std::move(stop), std::move(step))) {}

  PostfixExpr(LineRange pos, std::unique_ptr<FuncCall> func_call)
      : BaseAST(pos), expr(std::move(func_call)) {}

  PostfixExpr(LineRange pos, std::unique_ptr<FuncCall> func,
              std::unique_ptr<PostfixExpr> postfix)
      : BaseAST(pos), expr(FoldExpr(std::move(func), std::move(postfix))) {}

  PostfixExpr(LineRange pos, std::unique_ptr<PostfixExpr> postfix,
              PostfixOp op)
      : BaseAST(pos), expr(PostfixOpExpr(std::move(postfix), op)) {}

  PostfixExpr(LineRange pos, std::unique_ptr<PostfixExpr> postfix,
              std::string to_access)
      : BaseAST(pos), expr(MemberAccessExpr(std::move(postfix), to_access)) {}

  PostfixExpr(LineRange pos, std::vector<std::string> access)
      : BaseAST(pos), expr(MacroExpr(access)) {}

  json GetMetaData() const;
};

class UnaryExpr : public BaseAST {
  struct PrefixOpExpr {
    std::unique_ptr<UnaryExpr> rhs;
    PrefixOp op;

    PrefixOpExpr(std::unique_ptr<UnaryExpr> rhs, PrefixOp op)
        : rhs(std::move(rhs)), op(op) {}
  };

 public:
  std::variant<std::unique_ptr<PostfixExpr>, PrefixOpExpr> expr;

  UnaryExpr(LineRange pos, std::unique_ptr<PostfixExpr> expr)
      : expr(std::move(expr)), BaseAST(pos) {}
  UnaryExpr(LineRange pos, std::unique_ptr<UnaryExpr> unary, PrefixOp op)
      : expr(PrefixOpExpr(std::move(unary), op)), BaseAST(pos) {}

  json GetMetaData() const;
};

class PowerExpr : public BaseAST {
  struct OpExpr {
    std::unique_ptr<PowerExpr> lhs;
    std::unique_ptr<UnaryExpr> rhs;

    OpExpr(std::unique_ptr<PowerExpr> lhs,
           std::unique_ptr<UnaryExpr> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  };

 public:
  std::variant<std::unique_ptr<UnaryExpr>, OpExpr> expr;

  PowerExpr(LineRange pos, std::unique_ptr<UnaryExpr> expr)
      : expr(std::move(expr)), BaseAST(pos) {}
  PowerExpr(LineRange pos, std::unique_ptr<PowerExpr> lhs,
            std::unique_ptr<UnaryExpr> rhs)
      : expr(OpExpr(std::move(lhs), std::move(rhs))), BaseAST(pos) {}

  json GetMetaData() const;
};

class MultiplicativeExpr : public BaseAST {
  struct OpExpr {
    std::unique_ptr<MultiplicativeExpr> lhs;
    MultiplicativeOp op;
    std::unique_ptr<PowerExpr> rhs;

    OpExpr(std::unique_ptr<MultiplicativeExpr> lhs,
           MultiplicativeOp op, std::unique_ptr<PowerExpr> rhs)
        : lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}
  };

 public:
  std::variant<std::unique_ptr<PowerExpr>, OpExpr> expr;

  MultiplicativeExpr(LineRange pos, std::unique_ptr<PowerExpr> expr)
      : expr(std::move(expr)), BaseAST(pos) {}
  MultiplicativeExpr(LineRange pos,
                     std::unique_ptr<MultiplicativeExpr> lhs,
                     MultiplicativeOp op, std::unique_ptr<PowerExpr> rhs)
      : expr(OpExpr(std::move(lhs), op, std::move(rhs))), BaseAST(pos) {}

  json GetMetaData() const;
};

class AdditiveExpr : public BaseAST {
  struct OpExpr {
    std::unique_ptr<AdditiveExpr> lhs;
    AdditiveOp op;
    std::unique_ptr<MultiplicativeExpr> rhs;

    OpExpr(std::unique_ptr<AdditiveExpr> lhs, AdditiveOp op,
           std::unique_ptr<MultiplicativeExpr> rhs)
        : lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}
  };

 public:
  std::variant<std::unique_ptr<MultiplicativeExpr>, OpExpr> expr;

  AdditiveExpr(LineRange pos, std::unique_ptr<MultiplicativeExpr> expr)
      : expr(std::move(expr)), BaseAST(pos) {}
  AdditiveExpr(LineRange pos, std::unique_ptr<AdditiveExpr> lhs,
               AdditiveOp op, std::unique_ptr<MultiplicativeExpr> rhs)
      : expr(OpExpr(std::move(lhs), op, std::move(rhs))), BaseAST(pos) {}

  json GetMetaData() const;
};

class RelationalExpr : public BaseAST {
  struct OpExpr {
    std::unique_ptr<RelationalExpr> lhs;
    RelationalOp op;
    std::unique_ptr<AdditiveExpr> rhs;

    OpExpr(std::unique_ptr<RelationalExpr> lhs, RelationalOp op,
           std::unique_ptr<AdditiveExpr> rhs)
        : lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}
  };

 public:
  std::variant<std::unique_ptr<AdditiveExpr>, OpExpr> expr;

  RelationalExpr(LineRange pos, std::unique_ptr<AdditiveExpr> expr)
      : expr(std::move(expr)), BaseAST(pos) {}
  RelationalExpr(LineRange pos, std::unique_ptr<RelationalExpr> lhs,
                 RelationalOp op, std::unique_ptr<AdditiveExpr> rhs)
      : expr(OpExpr(std::move(lhs), op, std::move(rhs))),
        BaseAST(pos) {}

  json GetMetaData() const;
};

class EqualityExpr : public BaseAST {
  struct OpExpr {
    std::unique_ptr<EqualityExpr> lhs;
    EqualityOp op;
    std::unique_ptr<RelationalExpr> rhs;

    OpExpr(std::unique_ptr<EqualityExpr> lhs, EqualityOp op,
           std::unique_ptr<RelationalExpr> rhs)
        : lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}
  };

 public:
  std::variant<std::unique_ptr<RelationalExpr>, OpExpr> expr;

  EqualityExpr(LineRange pos, std::unique_ptr<RelationalExpr> expr)
      : expr(std::move(expr)), BaseAST(pos) {}
  EqualityExpr(LineRange pos, std::unique_ptr<EqualityExpr> lhs,
               EqualityOp op, std::unique_ptr<RelationalExpr> rhs)
      : expr(OpExpr(std::move(lhs), op, std::move(rhs))), BaseAST(pos) {}

  json GetMetaData() const;
};

class LogicalAndExpr : public BaseAST {
  struct OpExpr {
    std::unique_ptr<LogicalAndExpr> lhs;
    std::unique_ptr<EqualityExpr> rhs;

    OpExpr(std::unique_ptr<LogicalAndExpr> lhs,
           std::unique_ptr<EqualityExpr> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  };

 public:
  std::variant<std::unique_ptr<EqualityExpr>, OpExpr> expr;

  LogicalAndExpr(LineRange pos, std::unique_ptr<EqualityExpr> expr)
      : expr(std::move(expr)), BaseAST(pos) {}
  LogicalAndExpr(LineRange pos, std::unique_ptr<LogicalAndExpr> lhs,
                 std::unique_ptr<EqualityExpr> rhs)
      : expr(OpExpr(std::move(lhs), std::move(rhs))), BaseAST(pos) {}

  json GetMetaData() const;
};

class LogicalOrExpr : public BaseAST {
  struct OpExpr {
    std::unique_ptr<LogicalOrExpr> lhs;
    std::unique_ptr<LogicalAndExpr> rhs;

    OpExpr(std::unique_ptr<LogicalOrExpr> lhs,
           std::unique_ptr<LogicalAndExpr> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  };

 public:
  std::variant<std::unique_ptr<LogicalAndExpr>, OpExpr> expr;

  LogicalOrExpr(LineRange pos, std::unique_ptr<LogicalAndExpr> expr)
      : expr(std::move(expr)), BaseAST(pos) {}
  LogicalOrExpr(LineRange pos, std::unique_ptr<LogicalOrExpr> lhs,
                std::unique_ptr<LogicalAndExpr> rhs)
      : expr(OpExpr(std::move(lhs), std::move(rhs))), BaseAST(pos) {}

  json GetMetaData() const;
};

class ConditionalExpr : public BaseAST {
 public:
  std::unique_ptr<LogicalOrExpr> cond;
  std::optional<std::unique_ptr<Expr>> ternary_true;
  std::optional<std::unique_ptr<Expr>> ternary_false;

  ConditionalExpr(LineRange pos,
                        std::unique_ptr<LogicalOrExpr> expr)
      : cond(std::move(expr)), ternary_true(std::nullopt),
        ternary_false(std::nullopt), BaseAST(pos) {}
  
  ConditionalExpr(LineRange pos,
                  std::unique_ptr<LogicalOrExpr> expr,
                  std::unique_ptr<Expr> if_true, std::unique_ptr<Expr> if_false)
      : BaseAST(pos), cond(std::move(expr)), ternary_true(std::move(if_true)),
        ternary_false(std::move(if_false)) {}

  bool IsTernary() const { return this->ternary_true && this->ternary_false; }

  json GetMetaData() const;
};

class Block : public BaseAST {
 public:
  std::vector<std::unique_ptr<TopLevelExpr>> exprs;

  Block(LineRange pos, std::vector<std::unique_ptr<TopLevelExpr>> exprs)
      : BaseAST(pos), exprs(std::move(exprs)) {}

  json GetMetaData() const;
};

class Expr : public BaseAST {
  struct FuncBlock {
    std::unique_ptr<FuncDefinition> def;
    std::unique_ptr<Block> block;

    FuncBlock(std::unique_ptr<FuncDefinition> def,
              std::unique_ptr<Block> block)
        : def(std::move(def)), block(std::move(block)) {}
  };

  struct TupleBlock {
    std::unique_ptr<TupleDefinition> def;
    std::unique_ptr<Block> block;

    TupleBlock(std::unique_ptr<TupleDefinition> def,
               std::unique_ptr<Block> block)
        : def(std::move(def)), block(std::move(block)) {}
  };

 public:
  std::variant<std::unique_ptr<ConditionalExpr>, FuncBlock,
               TupleBlock, std::unique_ptr<ForLoop>,
               std::unique_ptr<WhileLoop>, std::unique_ptr<IfBlock>> expr;

  Expr(LineRange pos, std::unique_ptr<ConditionalExpr> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, std::unique_ptr<ForLoop> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, std::unique_ptr<WhileLoop> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, std::unique_ptr<IfBlock> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, std::unique_ptr<FuncDefinition> def,
       std::unique_ptr<Block> block)
      : BaseAST(pos), expr(FuncBlock(std::move(def), std::move(block))) {}
  Expr(LineRange pos, std::unique_ptr<TupleDefinition> def,
       std::unique_ptr<Block> block)
      : BaseAST(pos), expr(TupleBlock(std::move(def), std::move(block))) {}

  json GetMetaData() const;
};

class WhileLoop : public BaseAST {
 public:
  std::unique_ptr<ConditionalExpr> expr;
  std::unique_ptr<Block> block;
  std::optional<std::unique_ptr<Block>> else_block;

  WhileLoop(LineRange pos, std::unique_ptr<ConditionalExpr> expr,
            std::unique_ptr<Block> block)
      : BaseAST(pos), expr(std::move(expr)), block(std::move(block)),
        else_block(std::nullopt) {}

  WhileLoop(LineRange pos, std::unique_ptr<ConditionalExpr> expr,
            std::unique_ptr<Block> block, std::unique_ptr<Block> else_block)
      : BaseAST(pos), expr(std::move(expr)), block(std::move(block)),
        else_block(std::move(else_block)) {}

  json GetMetaData() const;
};

class ForLoopContents : public BaseAST {
  struct ForIn {
    std::vector<std::string> lhs;
    std::unique_ptr<ConditionalExpr> rhs;

    ForIn(std::vector<std::string> lhs,
          std::unique_ptr<ConditionalExpr> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  };

  struct ForTraditional {
    std::optional<std::unique_ptr<AssignmentExpr>> start;
    std::optional<std::unique_ptr<ConditionalExpr>> stop;
    std::optional<std::unique_ptr<ConditionalExpr>> step;

    ForTraditional(std::optional<std::unique_ptr<AssignmentExpr>> start,
                   std::optional<std::unique_ptr<ConditionalExpr>> stop,
                   std::optional<std::unique_ptr<ConditionalExpr>> step)
        : start(std::move(start)), stop(std::move(stop)),
          step(std::move(step)) {}
  };

 public:
  std::variant<ForIn, ForTraditional> expr;

  ForLoopContents(LineRange pos, std::vector<std::string> ids,
                  std::unique_ptr<ConditionalExpr> expr)
      : expr(ForIn(ids, std::move(expr))), BaseAST(pos) {}

  ForLoopContents(LineRange pos,
                  std::optional<std::unique_ptr<AssignmentExpr>> start,
                  std::optional<std::unique_ptr<ConditionalExpr>> stop,
                  std::optional<std::unique_ptr<ConditionalExpr>> step)
      : expr(ForTraditional(std::move(start), std::move(stop),
        std::move(step))), BaseAST(pos) {}

  json GetMetaData() const;
};

class ForLoop : public BaseAST {
 public:
  ForLoopContents expr;
  std::unique_ptr<Block> block;
  std::optional<std::unique_ptr<Block>> else_block;

  ForLoop(LineRange pos, ForLoopContents expr, std::unique_ptr<Block> block)
      : BaseAST(pos), expr(std::move(expr)), block(std::move(block)),
        else_block(std::nullopt) {}

  ForLoop(LineRange pos, ForLoopContents expr, std::unique_ptr<Block> block,
          std::unique_ptr<Block> else_block)
      : BaseAST(pos), expr(std::move(expr)), block(std::move(block)),
        else_block(std::move(else_block)) {}

  json GetMetaData() const;
};

class IfBlock : public BaseAST {
 public:
  std::unique_ptr<ConditionalExpr> cond;
  std::unique_ptr<Block> block;
  std::optional<std::unique_ptr<ElseBlock>> else_block;

  IfBlock(LineRange pos, std::unique_ptr<ConditionalExpr> cond,
          std::unique_ptr<Block> block)
      : BaseAST(pos), cond(std::move(cond)), block(std::move(block)) {}

  IfBlock(LineRange pos, std::unique_ptr<ConditionalExpr> cond,
          std::unique_ptr<Block> block, std::unique_ptr<ElseBlock> else_block)
      : BaseAST(pos), cond(std::move(cond)), block(std::move(block)),
        else_block(std::move(else_block)) {}

  json GetMetaData() const;
};

class ElseBlock : public BaseAST {
 public:
  std::variant<std::unique_ptr<IfBlock>, std::unique_ptr<Block>> expr;

  ElseBlock(LineRange pos, std::unique_ptr<IfBlock> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  ElseBlock(LineRange pos, std::unique_ptr<Block> arg)
      : expr(std::move(arg)), BaseAST(pos) {}

  json GetMetaData() const;
};

class TupleMember : public BaseAST {
 public:
  std::string id;
  std::optional<std::unique_ptr<TypeExpr>> type;

  TupleMember(LineRange pos, std::string id)
      : id(id), type(std::nullopt), BaseAST(pos) {}
  TupleMember(LineRange pos, std::string id,
              std::unique_ptr<TypeExpr> expr)
      : id(id), type(std::move(expr)), BaseAST(pos) {}

  json GetMetaData() const;
};

class TupleDefinition : public BaseAST {
 public:
  std::vector<std::unique_ptr<TupleMember>> members;

  TupleDefinition(LineRange pos,
                  std::vector<std::unique_ptr<TupleMember>> members)
      : BaseAST(pos), members(std::move(members)) {}

  json GetMetaData() const;
};

class TypeMember : public BaseAST {
  struct ArrayOrMapType {
    std::unique_ptr<TypeExpr> first_type;
    std::optional<std::unique_ptr<TypeExpr>> second_type;

    ArrayOrMapType(std::unique_ptr<TypeExpr> first_type,
                   std::optional<std::unique_ptr<TypeExpr>> second_type)
        : first_type(std::move(first_type)),
          second_type(std::move(second_type)) {}
  };

 public:
  std::variant<std::unique_ptr<FuncCall>, std::string,
               std::unique_ptr<FuncDefinition>,
               std::unique_ptr<TupleDefinition>, ArrayOrMapType> expr;

  TypeMember(LineRange pos, std::unique_ptr<FuncCall> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  TypeMember(LineRange pos, std::string arg) : expr(arg), BaseAST(pos) {}
  TypeMember(LineRange pos, std::unique_ptr<FuncDefinition> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
    TypeMember(LineRange pos, std::unique_ptr<TupleDefinition> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  TypeMember(LineRange pos, std::unique_ptr<TypeExpr> lhs,
             std::optional<std::unique_ptr<TypeExpr>> rhs)
      : BaseAST(pos), expr(ArrayOrMapType(std::move(lhs), std::move(rhs))) {}

  bool IsMap() const;

  json GetMetaData() const;
};

class TypeExpr : public BaseAST {
  struct ComplexExpr {
    std::unique_ptr<TypeExpr> lhs;
    std::optional<std::unique_ptr<TypeMember>> or_expr;
    std::optional<std::unique_ptr<TypeMember>> implements_expr;

    ComplexExpr(std::unique_ptr<TypeExpr> lhs,
                std::optional<std::unique_ptr<TypeMember>> or_expr,
                std::optional<std::unique_ptr<TypeMember>> implements_expr)
        : lhs(std::move(lhs)), or_expr(std::move(or_expr)),
          implements_expr(std::move(implements_expr)) {}
  };
 public:
  std::variant<std::unique_ptr<TypeMember>, ComplexExpr> expr;

  TypeExpr(LineRange pos, std::unique_ptr<TypeMember> expr)
      : BaseAST(pos), expr(std::move(expr)) {}
  TypeExpr(LineRange pos, std::unique_ptr<TypeExpr> lhs,
           std::optional<std::unique_ptr<TypeMember>> or_expr,
           std::optional<std::unique_ptr<TypeMember>> impl_expr)
      : BaseAST(pos), expr(ComplexExpr(std::move(lhs), std::move(or_expr),
        std::move(impl_expr))) {}

  json GetMetaData() const;
};

class FuncDefinition : public BaseAST {
 public:
  std::optional<std::unique_ptr<TupleDefinition>> args;
  std::optional<std::unique_ptr<TypeExpr>> ret_type;

  FuncDefinition(LineRange pos)
      : BaseAST(pos), args(std::nullopt), ret_type(std::nullopt) {}
  FuncDefinition(LineRange pos, std::unique_ptr<TupleDefinition> args)
      : BaseAST(pos), args(std::move(args)), ret_type(std::nullopt) {}
  FuncDefinition(LineRange pos, std::unique_ptr<TupleDefinition> args,
                 std::unique_ptr<TypeExpr> ret_type)
      : BaseAST(pos), args(std::move(args)), ret_type(std::move(ret_type)) {}

  json GetMetaData() const;
};

class Constant : public BaseAST {
 public:
  std::variant<std::unique_ptr<ArrayConstant>, std::unique_ptr<MapConstant>,
               double, i64, std::string, char> data;

  Constant(LineRange pos, std::unique_ptr<ArrayConstant> arg)
      : data(std::move(arg)), BaseAST(pos) {}
  Constant(LineRange pos, std::unique_ptr<MapConstant> arg)
      : data(std::move(arg)), BaseAST(pos) {}
  Constant(LineRange pos, double arg) : data(arg), BaseAST(pos) {}
  Constant(LineRange pos, i64 arg) : data(arg), BaseAST(pos) {}
  Constant(LineRange pos, std::string arg) : data(arg), BaseAST(pos) {}
  Constant(LineRange pos, char arg) : data(arg), BaseAST(pos) {}

  json GetMetaData() const;
};

class MapConstant : public BaseAST {
 public:
  std::vector<std::unique_ptr<Expr>> keys;
  std::vector<std::unique_ptr<Expr>> values;

  MapConstant(LineRange pos, std::vector<std::unique_ptr<Expr>> keys,
              std::vector<std::unique_ptr<Expr>> values)
      : BaseAST(pos), keys(std::move(keys)), values(std::move(values)) {}

  json GetMetaData() const;
};

class ArrayConstant : public BaseAST {
 public:
  std::vector<std::unique_ptr<Expr>> values;

  ArrayConstant(LineRange pos, std::vector<std::unique_ptr<Expr>> values)
      : BaseAST(pos), values(std::move(values)) {}

  json GetMetaData() const;
};

#endif