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
#include "token.hpp"

namespace porc::internals {

class BaseAST {
 protected:
  BaseAST(LineRange pos) : pos(pos) {}

 public:
  LineRange pos;

  BaseAST() = delete;

  virtual json GetMetaData() const = 0;
};

class FileDecl;
class FileBlock;
class AssignmentExpr;
class FuncBlock;
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
class VarDecl;
class Expr;
class WhileBlock;
class ForBlock;
class ElseBlock;
class IfBlock;
class TypeExpr;
class Constant;
class MacroExpr;
class TupleDecl;

/* TODO: when concepts finally come around this really could be useful here! 
   Something like:
enum_struct_operator AdditiveOp {
  enum Kind {
    ...
  };
}

For only 4+n (n = number of enum options) lines
we get the equivalent of 17+n!!!
*/

class AssignmentOp {
 public:
  enum Kind {
    AdditionEqual,
    SubtractionEqual,
    Equal,
    DivisionEqual,
    PowerEqual,
    ModulusEqual,
    MultiplyEqual,
    IntDivisionEqual,
  };

  AssignmentOp() = default;
  constexpr AssignmentOp(Kind kind) : value(kind) {}

  bool operator==(AssignmentOp a) const { return value == a.value; }
  bool operator!=(AssignmentOp a) const { return value != a.value; }

  const char *ToStr() const;
  static const char *AllMsg();
  static std::optional<AssignmentOp> FromToken(Token tok);

 private:
  Kind value;
};

class PostfixOp {
 public:
  enum Kind {
    Increment,
    Decrement,
  };

  PostfixOp() = default;
  constexpr PostfixOp(Kind kind) : value(kind) {}

  bool operator==(PostfixOp a) const { return value == a.value; }
  bool operator!=(PostfixOp a) const { return value != a.value; }

  const char *ToStr() const;
  static const char *AllMsg();
  static std::optional<PostfixOp> FromToken(Token tok);

 private:
  Kind value;
};

class PrefixOp {
 public:
  enum Kind {
    Negate,
    Negative,
    Positive,
    Increment,
    Decrement,
  };

  PrefixOp() = default;
  constexpr PrefixOp(Kind kind) : value(kind) {}

  bool operator==(PrefixOp a) const { return value == a.value; }
  bool operator!=(PrefixOp a) const { return value != a.value; }

  const char *ToStr() const;
  static const char *AllMsg();
  static std::optional<PrefixOp> FromToken(Token tok);

 private:
  Kind value;
};

class MultiplicativeOp {
 public:
  enum Kind {
    Multiplication,
    Division,
    Modulus,
    IntDivision,
  };

  MultiplicativeOp() = default;
  constexpr MultiplicativeOp(Kind kind) : value(kind) {}

  bool operator==(MultiplicativeOp a) const { return value == a.value; }
  bool operator!=(MultiplicativeOp a) const { return value != a.value; }

  const char *ToStr() const;
  static const char *AllMsg();
  static std::optional<MultiplicativeOp> FromToken(Token tok);

 private:
  Kind value;
};

class AdditiveOp {
 public:
  enum Kind {
    Subtraction,
    Addition,
  };

  AdditiveOp() = default;
  constexpr AdditiveOp(Kind kind) : value(kind) {}

  bool operator==(AdditiveOp a) const { return value == a.value; }
  bool operator!=(AdditiveOp a) const { return value != a.value; }

  const char *ToStr() const;
  static const char *AllMsg();
  static std::optional<AdditiveOp> FromToken(Token tok);

 private:
  Kind value;
};

class RelationalOp {
 public:
  enum Kind {
    GreaterThan,
    LessThan,
    GreaterThanEqual,
    LessThanEqual,
  };

  RelationalOp() = default;
  constexpr RelationalOp(Kind kind) : value(kind) {}

  bool operator==(RelationalOp a) const { return value == a.value; }
  bool operator!=(RelationalOp a) const { return value != a.value; }

  const char *ToStr() const;
  static const char *AllMsg();
  static std::optional<RelationalOp> FromToken(Token tok);

 private:
  Kind value;
};

class EqualityOp {
 public:
  enum Kind {
    Equal,
    NotEqual,
  };

  EqualityOp() = default;
  constexpr EqualityOp(Kind kind) : value(kind) {}

  bool operator==(EqualityOp a) const { return value == a.value; }
  bool operator!=(EqualityOp a) const { return value != a.value; }

  const char *ToStr() const;
  static const char *AllMsg();
  static std::optional<EqualityOp> FromToken(Token tok);

 private:
  Kind value;
};

class FileDecl : public BaseAST {
 public:
  std::vector<std::unique_ptr<FileBlock>> exprs;

  FileDecl(LineRange pos, std::vector<std::unique_ptr<FileBlock>> exprs)
      : exprs(std::move(exprs)), BaseAST(pos) {}

  json GetMetaData() const;
};

class VarDecl : public BaseAST {
 public:
  struct Declaration {
      bool is_const;
      std::string id;
      std::optional<std::unique_ptr<TypeExpr>> type;
      std::optional<std::unique_ptr<Expr>> expr;

      Declaration(bool is_const, std::string id,
                  std::unique_ptr<TypeExpr> type)
          : is_const(is_const), id(id), type(std::move(type)), expr(std::nullopt) {}

      Declaration(bool is_const, std::string id,
                  std::unique_ptr<Expr> expr)
          : is_const(is_const), id(id), expr(std::move(expr)), type(std::nullopt) {}

      Declaration(bool is_const, std::string id,
                  std::unique_ptr<TypeExpr> type,
                  std::unique_ptr<Expr> expr)
          : is_const(is_const), id(id), type(std::move(type)),
            expr(std::move(expr)) {}

      Declaration(bool is_const, std::string id,
                  std::optional<std::unique_ptr<TypeExpr>> type,
                  std::optional<std::unique_ptr<Expr>> expr)
          : is_const(is_const), id(id), type(std::move(type)),
            expr(std::move(expr)) {}
  };
  std::vector<Declaration> decls;

  VarDecl(LineRange pos, std::vector<Declaration> decls)
      : decls(std::move(decls)), BaseAST(pos) {}

  json GetMetaData() const;
};

class FileBlock : public BaseAST {
 public:
  std::variant<std::unique_ptr<Expr>, std::unique_ptr<VarDecl>,
               std::unique_ptr<MacroExpr>> expr;

  FileBlock(LineRange pos, std::unique_ptr<Expr> expr)
      : BaseAST(pos), expr(std::move(expr)) {}

  FileBlock(LineRange pos, std::unique_ptr<VarDecl> expr)
      : BaseAST(pos), expr(std::move(expr)) {}

  FileBlock(LineRange pos, std::unique_ptr<MacroExpr> expr)
      : BaseAST(pos), expr(std::move(expr)) {}

  json GetMetaData() const;
};

class FuncBlock : public BaseAST {
 public:
  std::variant<std::unique_ptr<AssignmentExpr>,
               std::unique_ptr<Expr>, std::unique_ptr<VarDecl>,
               std::unique_ptr<MacroExpr>,
               std::vector<std::unique_ptr<FuncBlock>>> expr;
  bool ret; // 'return' Expr (also true if no terminating ';')

  FuncBlock(LineRange pos, std::unique_ptr<AssignmentExpr> expr)
      : BaseAST(pos), ret(false), expr(std::move(expr)) {}

  FuncBlock(LineRange pos, std::unique_ptr<Expr> expr, bool ret)
      : BaseAST(pos), ret(ret), expr(std::move(expr)) {}

  FuncBlock(LineRange pos, std::unique_ptr<VarDecl> expr, bool ret)
      : BaseAST(pos), ret(ret), expr(std::move(expr)) {}

  FuncBlock(LineRange pos, std::unique_ptr<MacroExpr> expr, bool ret)
      : BaseAST(pos), ret(ret), expr(std::move(expr)) {}

  FuncBlock(LineRange pos, std::vector<std::unique_ptr<FuncBlock>> exprs,
            bool ret)
      : BaseAST(pos), ret(ret), expr(std::move(exprs)) {}

  json GetMetaData() const;
};

class TupleDecl : public BaseAST {
 public:
  struct ArgDecl {
    std::optional<std::unique_ptr<TypeExpr>> type;
    bool is_const;
    std::string id;
    std::optional<std::unique_ptr<Expr>> expr;

    ArgDecl(bool is_const, std::string id, std::unique_ptr<TypeExpr> type,
            std::unique_ptr<Expr> expr)
        : id(id), type(std::move(type)), is_const(is_const),
          expr(std::move(expr)) {}
    ArgDecl(bool is_const, std::string id, std::unique_ptr<Expr> expr)
        : id(id), type(std::nullopt), is_const(is_const),
          expr(std::move(expr)) {}
    ArgDecl(std::unique_ptr<TypeExpr> type, bool is_const, std::string id)
        : id(id), type(std::move(type)), is_const(is_const),
          expr(std::nullopt) {}
    ArgDecl(bool is_const, std::string id)
        : id(id), type(std::nullopt), is_const(is_const),
          expr(std::nullopt) {}
    ArgDecl(bool is_const, std::string id,
            std::optional<std::unique_ptr<TypeExpr>> type,
            std::optional<std::unique_ptr<Expr>> expr)
        : id(id), type(std::move(type)), is_const(is_const),
          expr(std::move(expr)) {}
  };

  std::vector<ArgDecl> args;

  TupleDecl(LineRange pos, std::vector<ArgDecl> args)
      : args(std::move(args)), BaseAST(pos) {}

  json GetMetaData() const;
};

class MacroExpr : public BaseAST {
 public:
  std::vector<std::string> qualifying_name;
  std::vector<std::unique_ptr<Expr>> args;

  MacroExpr(LineRange pos, std::vector<std::string> qualifying_name,
            std::vector<std::unique_ptr<Expr>> args)
      : BaseAST(pos), qualifying_name(qualifying_name), args(std::move(args)) {}

  json GetMetaData() const;
};

class AssignmentExpr : public BaseAST {
 public:
  std::vector<std::unique_ptr<Expr>> lhs;
  AssignmentOp op;
  std::vector<std::unique_ptr<Expr>> rhs;

  AssignmentExpr(LineRange pos, std::vector<std::unique_ptr<Expr>> lhs, 
                 AssignmentOp op, std::vector<std::unique_ptr<Expr>> rhs)
      : BaseAST(pos), lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}

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
 public:
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

  std::variant<IndexExpr, SliceExpr, std::unique_ptr<FuncCall>, FoldExpr,
               PostfixOpExpr, std::unique_ptr<Expr>, std::unique_ptr<TypeExpr>,
               MemberAccessExpr, std::unique_ptr<MacroExpr>,
               std::unique_ptr<Constant>, std::string> expr;

  PostfixExpr(LineRange pos, std::string expr) 
      : BaseAST(pos), expr(expr) {}

  PostfixExpr(LineRange pos, std::unique_ptr<TypeExpr> expr) 
      : BaseAST(pos), expr(std::move(expr)) {}

  PostfixExpr(LineRange pos, std::unique_ptr<Expr> expr) 
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

  PostfixExpr(LineRange pos, std::unique_ptr<MacroExpr> expr)
      : BaseAST(pos), expr(std::move(expr)) {}

  PostfixExpr(LineRange pos, std::unique_ptr<Constant> expr)
      : BaseAST(pos), expr(std::move(expr)) {}

  json GetMetaData() const;
};

class UnaryExpr : public BaseAST {
 public:
  struct PrefixOpExpr {
    std::unique_ptr<UnaryExpr> rhs;
    PrefixOp op;

    PrefixOpExpr(std::unique_ptr<UnaryExpr> rhs, PrefixOp op)
        : rhs(std::move(rhs)), op(op) {}
  };

  std::variant<std::unique_ptr<PostfixExpr>, PrefixOpExpr> expr;

  UnaryExpr(LineRange pos, std::unique_ptr<PostfixExpr> expr)
      : expr(std::move(expr)), BaseAST(pos) {}
  UnaryExpr(LineRange pos, std::unique_ptr<UnaryExpr> unary, PrefixOp op)
      : expr(PrefixOpExpr(std::move(unary), op)), BaseAST(pos) {}

  json GetMetaData() const;
};

class PowerExpr : public BaseAST {
 public:
  std::vector<std::unique_ptr<UnaryExpr>> exprs;

  PowerExpr(LineRange pos,
            std::unique_ptr<UnaryExpr> expr)
      : BaseAST(pos) { exprs.push_back(std::move(expr)); }

  PowerExpr(LineRange pos, std::vector<std::unique_ptr<UnaryExpr>> exprs)
      : exprs(std::move(exprs)), BaseAST(pos) {}

  json GetMetaData() const;
};

class MultiplicativeExpr : public BaseAST {
 public:
  struct OpExpr {
    MultiplicativeOp op;
    std::unique_ptr<PowerExpr> rhs;

    OpExpr(MultiplicativeOp op, std::unique_ptr<PowerExpr> rhs)
        : op(op), rhs(std::move(rhs)) {}
  };

  std::unique_ptr<PowerExpr> lhs;
  std::vector<OpExpr> exprs;

  MultiplicativeExpr(LineRange pos, std::unique_ptr<PowerExpr> fallthrough)
      : lhs(std::move(fallthrough)), BaseAST(pos) {}
  MultiplicativeExpr(LineRange pos, std::unique_ptr<PowerExpr> lhs,
               std::vector<OpExpr> exprs)
      : exprs(std::move(exprs)), lhs(std::move(lhs)), BaseAST(pos) {}

  json GetMetaData() const;
};

class AdditiveExpr : public BaseAST {
 public:
  struct OpExpr {
    AdditiveOp op;
    std::unique_ptr<MultiplicativeExpr> rhs;

    OpExpr(AdditiveOp op, std::unique_ptr<MultiplicativeExpr> rhs)
        : op(op), rhs(std::move(rhs)) {}
  };

  std::unique_ptr<MultiplicativeExpr> lhs;
  std::vector<OpExpr> exprs;

  AdditiveExpr(LineRange pos, std::unique_ptr<MultiplicativeExpr> fallthrough)
      : lhs(std::move(fallthrough)), BaseAST(pos) {}
  AdditiveExpr(LineRange pos, std::unique_ptr<MultiplicativeExpr> lhs,
               std::vector<OpExpr> exprs)
      : exprs(std::move(exprs)), lhs(std::move(lhs)), BaseAST(pos) {}

  json GetMetaData() const;
};

class RelationalExpr : public BaseAST {
 public:
  struct OpExpr {
    RelationalOp op;
    std::unique_ptr<AdditiveExpr> rhs;

    OpExpr(RelationalOp op, std::unique_ptr<AdditiveExpr> rhs)
        : op(op), rhs(std::move(rhs)) {}
  };

  std::unique_ptr<AdditiveExpr> lhs;
  std::vector<OpExpr> exprs;

  RelationalExpr(LineRange pos, std::unique_ptr<AdditiveExpr> fallthrough)
      : lhs(std::move(fallthrough)), BaseAST(pos) {}
  RelationalExpr(LineRange pos, std::unique_ptr<AdditiveExpr> lhs,
                 std::vector<OpExpr> exprs)
      : exprs(std::move(exprs)), lhs(std::move(lhs)), BaseAST(pos) {}

  json GetMetaData() const;
};

class EqualityExpr : public BaseAST {
 public:
  struct OpExpr {
    EqualityOp op;
    std::unique_ptr<RelationalExpr> rhs;

    OpExpr(EqualityOp op, std::unique_ptr<RelationalExpr> rhs)
        : op(op), rhs(std::move(rhs)) {}
  };

  std::unique_ptr<RelationalExpr> lhs;
  std::vector<OpExpr> exprs;

  EqualityExpr(LineRange pos, std::unique_ptr<RelationalExpr> fallthrough)
      : lhs(std::move(fallthrough)), BaseAST(pos) {}
  EqualityExpr(LineRange pos, std::unique_ptr<RelationalExpr> lhs,
               std::vector<OpExpr> exprs)
      : exprs(std::move(exprs)), lhs(std::move(lhs)), BaseAST(pos) {}

  json GetMetaData() const;
};

class LogicalAndExpr : public BaseAST {
 public:
  std::vector<std::unique_ptr<EqualityExpr>> exprs;

  LogicalAndExpr(LineRange pos,
                 std::unique_ptr<EqualityExpr> expr)
      : BaseAST(pos) { exprs.push_back(std::move(expr)); }

  LogicalAndExpr(LineRange pos,
                 std::vector<std::unique_ptr<EqualityExpr>> exprs)
      : exprs(std::move(exprs)), BaseAST(pos) {}

  json GetMetaData() const;
};

class LogicalOrExpr : public BaseAST {
 public:
  std::vector<std::unique_ptr<LogicalAndExpr>> exprs;

  LogicalOrExpr(LineRange pos,
                std::unique_ptr<LogicalAndExpr> expr)
      : BaseAST(pos) { exprs.push_back(std::move(expr)); }

  LogicalOrExpr(LineRange pos,
                std::vector<std::unique_ptr<LogicalAndExpr>> exprs)
      : exprs(std::move(exprs)), BaseAST(pos) {}

  json GetMetaData() const;
};

class Expr : public BaseAST {
 public:
  struct FuncDecl {
    std::unique_ptr<TupleDecl> args;
    std::optional<std::unique_ptr<TypeExpr>> ret_type;
    std::unique_ptr<Expr> block;

    FuncDecl(std::unique_ptr<TupleDecl> args,
             std::optional<std::unique_ptr<TypeExpr>> ret_type,
             std::unique_ptr<Expr> block)
        : args(std::move(args)), block(std::move(block)),
          ret_type(std::move(ret_type)) {}
  };

  struct StructDecl {
    std::unique_ptr<TupleDecl> members;
    std::vector<std::unique_ptr<FileBlock>> block;

    StructDecl(std::unique_ptr<TupleDecl> members,
               std::vector<std::unique_ptr<FileBlock>> block)
        : members(std::move(members)), block(std::move(block)) {}
  };

  struct RangeExpr {
    std::unique_ptr<LogicalOrExpr> start;
    std::unique_ptr<LogicalOrExpr> stop;
    std::optional<std::unique_ptr<LogicalOrExpr>> step;
    bool inclusive;

    RangeExpr(std::unique_ptr<LogicalOrExpr> start,
              std::unique_ptr<LogicalOrExpr> stop,
              std::optional<std::unique_ptr<LogicalOrExpr>> step, bool inclusive)
        : start(std::move(start)), stop(std::move(stop)), step(std::move(step)),
          inclusive(inclusive) {}
  };

  struct MapExpr {
    std::vector<std::unique_ptr<Expr>> keys;
    std::vector<std::unique_ptr<Expr>> values;

    MapExpr(std::vector<std::unique_ptr<Expr>> keys,
            std::vector<std::unique_ptr<Expr>> values)
        : keys(std::move(keys)), values(std::move(values)) {}
  };

  struct CollectionExpr {
   private:
    bool collection_is_array;
   public:
    std::vector<std::unique_ptr<Expr>> values;
    bool is_array() const { return this->collection_is_array; };
    bool is_tuple() const { return !this->collection_is_array; }

    CollectionExpr(std::vector<std::unique_ptr<Expr>> values,
              bool is_array)
        : values(std::move(values)), collection_is_array(is_array) {}
  };

  std::variant<std::unique_ptr<LogicalOrExpr>, FuncDecl,
               StructDecl, std::unique_ptr<VarDecl>, RangeExpr,
               std::unique_ptr<ForBlock>, std::unique_ptr<WhileBlock>,
               std::unique_ptr<IfBlock>, MapExpr, CollectionExpr,
               std::vector<std::unique_ptr<FuncBlock>>> expr;

  Expr(LineRange pos, std::unique_ptr<LogicalOrExpr> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, std::unique_ptr<VarDecl> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, bool inclusive, std::unique_ptr<LogicalOrExpr> start,
       std::unique_ptr<LogicalOrExpr> stop,
       std::optional<std::unique_ptr<LogicalOrExpr>> step)
      : BaseAST(pos),
        expr(RangeExpr(std::move(start), std::move(stop), std::move(step),
                  inclusive)) {}
  Expr(LineRange pos, std::unique_ptr<TupleDecl> members,
       std::vector<std::unique_ptr<FileBlock>> block)
      : BaseAST(pos), expr(StructDecl(std::move(members), std::move(block))) {}
  Expr(LineRange pos, std::unique_ptr<TupleDecl> args,
       std::optional<std::unique_ptr<TypeExpr>> ret_type,
       std::unique_ptr<Expr> block)
      : BaseAST(pos), expr(FuncDecl(std::move(args), std::move(ret_type),
                           std::move(block))) {}
  Expr(LineRange pos, std::vector<std::unique_ptr<Expr>> members, bool is_array)
      : BaseAST(pos), expr(CollectionExpr(std::move(members), is_array)) {}
  Expr(LineRange pos, std::vector<std::unique_ptr<Expr>> keys,
       std::vector<std::unique_ptr<Expr>> values)
      : BaseAST(pos), expr(MapExpr(std::move(keys), std::move(values))) {}
  Expr(LineRange pos, std::unique_ptr<IfBlock> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, std::unique_ptr<WhileBlock> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, std::unique_ptr<ForBlock> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, std::vector<std::unique_ptr<FuncBlock>> block)
      : expr(std::move(block)), BaseAST(pos) {}

  json GetMetaData() const;
};

class WhileBlock : public BaseAST {
 public:
  std::unique_ptr<Expr> expr;
  std::vector<std::unique_ptr<FuncBlock>> block;

  WhileBlock(LineRange pos, std::unique_ptr<Expr> expr,
            std::vector<std::unique_ptr<FuncBlock>> block)
      : BaseAST(pos), expr(std::move(expr)), block(std::move(block)) {}

  json GetMetaData() const;
};

class ForBlock : public BaseAST {
 public:
  std::vector<std::string> id_list;
  std::vector<std::unique_ptr<Expr>> expr_list;
  std::vector<std::unique_ptr<FuncBlock>> block;

  ForBlock(LineRange pos, std::vector<std::string> id_list,
           std::vector<std::unique_ptr<Expr>> expr_list,
           std::vector<std::unique_ptr<FuncBlock>> block)
      : BaseAST(pos), id_list(std::move(id_list)),
        expr_list(std::move(expr_list)), block(std::move(block)) {}

  json GetMetaData() const;
};

class IfBlock : public BaseAST {
 public:
  struct IfStatement {
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Expr> block;

    IfStatement(std::unique_ptr<Expr> cond,
                std::unique_ptr<Expr> block)
        : cond(std::move(cond)), block(std::move(block)) {}
  };

  std::vector<IfStatement> statements; // series of if { } else if { } ...
  std::optional<std::unique_ptr<Expr>> else_block; // else {}

  IfBlock(LineRange pos, std::vector<IfStatement> statements)
      : BaseAST(pos), statements(std::move(statements)),
        else_block(std::nullopt) {}

  IfBlock(LineRange pos, std::vector<IfStatement> statements,
          std::unique_ptr<Expr> else_block)
      : BaseAST(pos), statements(std::move(statements)),
        else_block(std::move(else_block)) {}

  json GetMetaData() const;
};

class TypeExpr : public BaseAST {
 public:
  struct CollectionType {
    std::unique_ptr<TypeExpr> first_type;
    // if `int` == -1 then it is growable
    std::vector<std::variant<std::unique_ptr<TypeExpr>, int>> second_type;

    CollectionType(std::unique_ptr<TypeExpr> first_type,
                   std::vector<std::variant<std::unique_ptr<TypeExpr>, int>> t2)
        : first_type(std::move(first_type)),
          second_type(std::move(t2)) {}
  };

  struct VariantType {
    std::unique_ptr<TypeExpr> lhs;
    std::vector<std::unique_ptr<TypeExpr>> rhs;

    VariantType(std::unique_ptr<TypeExpr> lhs,
                std::vector<std::unique_ptr<TypeExpr>> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  };

  struct TupleType {
    struct TypedTupleArg {
      bool is_const;
      std::optional<std::string> id;
      std::unique_ptr<TypeExpr> expr;

      TypedTupleArg(bool is_const, std::string id,
                    std::unique_ptr<TypeExpr> expr)
          : is_const(is_const), id(id), expr(std::move(expr)) {}

      TypedTupleArg(bool is_const, std::unique_ptr<TypeExpr> expr)
          : is_const(is_const), id(std::nullopt), expr(std::move(expr)) {}
    };

    std::vector<TypedTupleArg> types;

    TupleType(std::vector<TypedTupleArg> types): types(std::move(types)) {}
    TupleType(void) {}
  };

  struct FunctionType {
    std::optional<std::string> id;
    std::unique_ptr<TupleDecl> args;
    std::optional<std::unique_ptr<TypeExpr>> ret_type;

    FunctionType(std::string id, std::unique_ptr<TupleDecl> args,
                 std::unique_ptr<TypeExpr> ret_type)
        : id(id), args(std::move(args)), ret_type(std::move(ret_type)) {}

    FunctionType(std::unique_ptr<TupleDecl> args,
                 std::unique_ptr<TypeExpr> ret_type)
        : id(std::nullopt), args(std::move(args)),
          ret_type(std::move(ret_type)) {}

    FunctionType(std::string id, std::unique_ptr<TupleDecl> args)
        : id(id), args(std::move(args)), ret_type(std::nullopt) {}

    FunctionType(std::unique_ptr<TupleDecl> args)
        : id(std::nullopt), args(std::move(args)), ret_type(std::nullopt) {}
  };

  std::variant<CollectionType, VariantType, std::vector<std::string>, TupleType,
               FunctionType> expr;

  TypeExpr(LineRange pos, std::vector<std::string> id)
      : BaseAST(pos), expr(std::move(id)) {}

  TypeExpr(LineRange pos, FunctionType func_type)
      : BaseAST(pos), expr(std::move(func_type)) {}

  TypeExpr(LineRange pos, VariantType variant_type)
      : BaseAST(pos), expr(std::move(variant_type)) {}

  TypeExpr(LineRange pos, CollectionType collection_type)
      : BaseAST(pos), expr(std::move(collection_type)) {}

  TypeExpr(LineRange pos, TupleType tuple_type)
      : BaseAST(pos), expr(std::move(tuple_type)) {}

  json GetMetaData() const;
};

class Constant : public BaseAST {
 public:
  std::variant<double, i64, std::string, char, bool> data;

  Constant(LineRange pos, double arg) : data(arg), BaseAST(pos) {}
  Constant(LineRange pos, i64 arg) : data(arg), BaseAST(pos) {}
  Constant(LineRange pos, std::string arg) : data(arg), BaseAST(pos) {}
  Constant(LineRange pos, char arg) : data(arg), BaseAST(pos) {}
  Constant(LineRange pos, bool arg) : data(arg), BaseAST(pos) {}

  json GetMetaData() const;
};

}

#endif