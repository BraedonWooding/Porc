#ifndef AST_HPP
#define AST_HPP

// This is predominantly based upon the EBNF
// of course changes will occur
// i.e. ArgumentExprList => Vector(Expr)

/*
  NOTE: often we will do a few transformations here to make it much flatter
        easing our workflow, for example while blocks will have the form;
        'while' expr func_block* effectively instead of having the form
        where it contains an expression since they are identical but it
        can help in the case of `{ }` to flatten it by a level
  This does introduce ambiguity in terms of our output for example;
  - `x => y` is identical (in our output) to `x => { y }` but in terms of EBNF
    they are different.  However this really should have no impact.
*/

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

class IdentifierAccess;
class FileDecl;
class StructBlock;
class AssignmentExpr;
class FuncBlock;
class FuncCall;
class Atom;
class UnaryExpr;
class PowerExpr;
class MultiplicativeExpr;
class AdditiveExpr;
class ComparisonExpr;
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

class PrefixOp {
 public:
  enum Kind {
    Negate,
    Negative,
    Positive,
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

class ComparisonOp {
 public:
  enum Kind {
    GreaterThan,
    LessThan,
    GreaterThanEqual,
    LessThanEqual,
    Equal,
    NotEqual,
  };

  ComparisonOp() = default;
  constexpr ComparisonOp(Kind kind) : value(kind) {}

  bool operator==(ComparisonOp a) const { return value == a.value; }
  bool operator!=(ComparisonOp a) const { return value != a.value; }

  const char *ToStr() const;
  static const char *AllMsg();
  static std::optional<ComparisonOp> FromToken(Token tok);

 private:
  Kind value;
};

class IdentifierAccess : public BaseAST {
 public:
  std::vector<LineStr> idents;

  IdentifierAccess(LineRange pos, std::vector<LineStr> idents)
      : idents(std::move(idents)), BaseAST(pos) {}

  json GetMetaData() const;
};

class FileDecl : public BaseAST {
 public:
  std::vector<std::unique_ptr<FuncBlock>> exprs;

  FileDecl(LineRange pos, std::vector<std::unique_ptr<FuncBlock>> exprs)
      : exprs(std::move(exprs)), BaseAST(pos) {}

  json GetMetaData() const;
};

class VarDecl : public BaseAST {
 public:
  struct Declaration {
      LineStr id;
      std::optional<std::unique_ptr<TypeExpr>> type;
      std::optional<std::unique_ptr<Expr>> expr;

      LineRange GetPos() const;

      Declaration(LineStr id,
                  std::optional<std::unique_ptr<TypeExpr>> type,
                  std::optional<std::unique_ptr<Expr>> expr)
          : id(id), type(std::move(type)), expr(std::move(expr)) {}
  };
  std::vector<Declaration> decls;
  bool is_mut;

  VarDecl(LineRange pos, bool is_mut, std::vector<Declaration> decls)
      : decls(std::move(decls)), is_mut(is_mut), BaseAST(pos) {}

  json GetMetaData() const;
};

// @TODO: Expr is only valid for func/struct decls
class StructBlock : public BaseAST {
 public:
  std::variant<std::unique_ptr<Expr>, std::unique_ptr<VarDecl>,
               std::unique_ptr<MacroExpr>> expr;

  StructBlock(LineRange pos, std::unique_ptr<Expr> expr)
      : BaseAST(pos), expr(std::move(expr)) {}

  StructBlock(LineRange pos, std::unique_ptr<VarDecl> expr)
      : BaseAST(pos), expr(std::move(expr)) {}

  StructBlock(LineRange pos, std::unique_ptr<MacroExpr> expr)
      : BaseAST(pos), expr(std::move(expr)) {}

  json GetMetaData() const;
};

class FuncBlock : public BaseAST {
 public:
  std::variant<std::unique_ptr<AssignmentExpr>,
               std::unique_ptr<Expr>, std::unique_ptr<VarDecl>> expr;
  bool ret;

  FuncBlock(LineRange pos, std::unique_ptr<AssignmentExpr> expr,
            bool ret = false)
      : BaseAST(pos), expr(std::move(expr)), ret(ret) {}

  FuncBlock(LineRange pos, std::unique_ptr<Expr> expr, bool ret = false)
      : BaseAST(pos), expr(std::move(expr)), ret(ret) {}

  FuncBlock(LineRange pos, std::unique_ptr<VarDecl> expr, bool ret = false)
      : BaseAST(pos), expr(std::move(expr)), ret(ret) {}

  json GetMetaData() const;
};

class TupleDecl : public BaseAST {
 public:
  struct ArgDecl {
    std::optional<std::unique_ptr<TypeExpr>> type;
    bool is_mut;
    LineStr id;
    bool generic_id;
    std::optional<std::unique_ptr<Expr>> expr;

    ArgDecl(bool is_mut, bool generic, LineStr id,
            std::optional<std::unique_ptr<TypeExpr>> type,
            std::optional<std::unique_ptr<Expr>> expr)
        : id(id), generic_id(generic), type(std::move(type)), is_mut(is_mut),
          expr(std::move(expr)) {}
  };

  std::vector<ArgDecl> args;

  TupleDecl(LineRange pos, std::vector<ArgDecl> args)
      : args(std::move(args)), BaseAST(pos) {}

  json GetMetaData() const;
};

class MacroExpr : public BaseAST {
 public:
  IdentifierAccess qualifying_name;
  std::vector<std::unique_ptr<Expr>> args;

  MacroExpr(LineRange pos, IdentifierAccess qualifying_name,
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
  std::unique_ptr<Atom> func;
  std::vector<std::unique_ptr<Expr>> args;

  FuncCall(LineRange pos, std::unique_ptr<Atom> expr,
           std::vector<std::unique_ptr<Expr>> args)
      : BaseAST(pos), func(std::move(expr)), args(std::move(args)) {}

  json GetMetaData() const;
};

class Atom : public BaseAST {
 public:
  struct IndexExpr {
    std::unique_ptr<Atom> lhs;
    std::unique_ptr<Expr> index;

    IndexExpr(std::unique_ptr<Atom> lhs,
              std::unique_ptr<Expr> index)
        : lhs(std::move(lhs)), index(std::move(index)) {}
  };

  struct SliceExpr {
    std::unique_ptr<Atom> obj;
    std::optional<std::unique_ptr<Expr>> start;
    std::optional<std::unique_ptr<Expr>> stop;
    std::optional<std::unique_ptr<Expr>> step;

    SliceExpr(std::unique_ptr<Atom> obj,
              std::optional<std::unique_ptr<Expr>> start,
              std::optional<std::unique_ptr<Expr>> stop,
              std::optional<std::unique_ptr<Expr>> step)
        : obj(std::move(obj)), start(std::move(start)), stop(std::move(stop)),
          step(std::move(step)) {}
  };

  struct FoldExpr {
    std::unique_ptr<FuncCall> func;
    std::unique_ptr<Expr> fold_expr;
    bool folds_right; // |> vs <|

    FoldExpr(std::unique_ptr<FuncCall> func, bool folds_right,
             std::unique_ptr<Expr> fold_expr)
        : func(std::move(func)), fold_expr(std::move(fold_expr)),
          folds_right(folds_right) {}
  };

  struct MemberAccessExpr {
    std::unique_ptr<Atom> lhs;
    IdentifierAccess access;

    MemberAccessExpr(std::unique_ptr<Atom> lhs, IdentifierAccess access)
        : lhs(std::move(lhs)), access(access) {}
  };

  std::variant<IndexExpr, SliceExpr, std::unique_ptr<FuncCall>, FoldExpr,
               std::unique_ptr<Expr>, MemberAccessExpr,
               std::unique_ptr<MacroExpr>, std::unique_ptr<Constant>,
               LineStr> expr;

  Atom(LineStr id) 
      : BaseAST(id.pos), expr(id) {}

  Atom(std::unique_ptr<Expr> expr) 
      : BaseAST(expr->pos), expr(std::move(expr)) {}

  Atom(std::unique_ptr<Atom> lhs,
       std::unique_ptr<Expr> index)
      : BaseAST(LineRange(lhs->pos,index->pos)),
        expr(IndexExpr(std::move(lhs), std::move(index))) {}

  Atom(LineRange pos, std::unique_ptr<Atom> lhs,
       std::optional<std::unique_ptr<Expr>> start,
       std::optional<std::unique_ptr<Expr>> stop,
       std::optional<std::unique_ptr<Expr>> step)
      : BaseAST(pos), expr(SliceExpr(std::move(lhs), std::move(start),
        std::move(stop), std::move(step))) {}

  Atom(std::unique_ptr<FuncCall> func_call)
      : BaseAST(func_call->pos), expr(std::move(func_call)) {}

  Atom(LineRange pos, std::unique_ptr<FuncCall> func, bool folds_right,
       std::unique_ptr<Expr> rhs)
      : BaseAST(pos), expr(FoldExpr(std::move(func), folds_right,
                           std::move(rhs))) {}

  Atom(LineRange pos, std::unique_ptr<Atom> lhs,
       IdentifierAccess access)
      : BaseAST(pos), expr(MemberAccessExpr(std::move(lhs), access)) {}

  Atom(std::unique_ptr<MacroExpr> expr)
      : BaseAST(expr->pos), expr(std::move(expr)) {}

  Atom(std::unique_ptr<Constant> expr)
      : BaseAST(expr->pos), expr(std::move(expr)) {}

  json GetMetaData() const;
};

class PowerExpr : public BaseAST {
 public:
  std::vector<std::unique_ptr<Atom>> exprs;

  PowerExpr(LineRange pos,
            std::unique_ptr<Atom> expr)
      : BaseAST(pos) { exprs.push_back(std::move(expr)); }

  PowerExpr(LineRange pos, std::vector<std::unique_ptr<Atom>> exprs)
      : exprs(std::move(exprs)), BaseAST(pos) {}

  json GetMetaData() const;
};

class UnaryExpr : public BaseAST {
  std::unique_ptr<PowerExpr> rhs;
  std::vector<PrefixOp> ops;

  UnaryExpr(std::unique_ptr<PowerExpr> expr)
      : BaseAST(expr->pos), rhs(std::move(expr)) {}

  UnaryExpr(LineRange pos, std::unique_ptr<PowerExpr> expr,
            std::vector<PrefixOp> ops)
      : BaseAST(pos), rhs(std::move(expr)), ops(ops) {}

  json GetMetaData() const;
};

class MultiplicativeExpr : public BaseAST {
 public:
  struct OpExpr {
    MultiplicativeOp op;
    std::unique_ptr<UnaryExpr> rhs;

    OpExpr(MultiplicativeOp op, std::unique_ptr<UnaryExpr> rhs)
        : op(op), rhs(std::move(rhs)) {}
  };

  std::unique_ptr<UnaryExpr> lhs;
  std::vector<OpExpr> exprs;

  MultiplicativeExpr(LineRange pos, std::unique_ptr<UnaryExpr> fallthrough)
      : lhs(std::move(fallthrough)), BaseAST(pos) {}
  MultiplicativeExpr(LineRange pos, std::unique_ptr<UnaryExpr> lhs,
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

class ComparisonExpr : public BaseAST {
 public:
  struct OpExpr {
    ComparisonOp op;
    std::unique_ptr<AdditiveExpr> rhs;

    OpExpr(ComparisonOp op, std::unique_ptr<AdditiveExpr> rhs)
        : op(op), rhs(std::move(rhs)) {}
  };

  std::unique_ptr<AdditiveExpr> lhs;
  std::vector<OpExpr> exprs;

  ComparisonExpr(LineRange pos, std::unique_ptr<AdditiveExpr> fallthrough)
      : lhs(std::move(fallthrough)), BaseAST(pos) {}
  ComparisonExpr(LineRange pos, std::unique_ptr<AdditiveExpr> lhs,
                 std::vector<OpExpr> exprs)
      : exprs(std::move(exprs)), lhs(std::move(lhs)), BaseAST(pos) {}

  json GetMetaData() const;
};

class LogicalAndExpr : public BaseAST {
 public:
  std::vector<std::unique_ptr<ComparisonExpr>> exprs;

  LogicalAndExpr(LineRange pos,
                 std::unique_ptr<ComparisonExpr> expr)
      : BaseAST(pos) { exprs.push_back(std::move(expr)); }

  LogicalAndExpr(LineRange pos,
                 std::vector<std::unique_ptr<ComparisonExpr>> exprs)
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
    std::vector<std::unique_ptr<FuncBlock>> block;

    FuncDecl(std::unique_ptr<TupleDecl> args,
             std::optional<std::unique_ptr<TypeExpr>> ret_type,
             std::vector<std::unique_ptr<FuncBlock>> block)
        : args(std::move(args)), block(std::move(block)),
          ret_type(std::move(ret_type)) {}
  };

  struct StructDecl {
    std::unique_ptr<TupleDecl> members;
    std::vector<std::unique_ptr<StructBlock>> block;

    StructDecl(std::unique_ptr<TupleDecl> members,
               std::vector<std::unique_ptr<StructBlock>> block)
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
       std::vector<std::unique_ptr<StructBlock>> block)
      : BaseAST(pos), expr(StructDecl(std::move(members), std::move(block))) {}
  Expr(LineRange pos, std::unique_ptr<TupleDecl> args,
       std::optional<std::unique_ptr<TypeExpr>> ret_type,
       std::vector<std::unique_ptr<FuncBlock>> block)
      : BaseAST(pos),
        expr(FuncDecl(std::move(args), std::move(ret_type),
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
  IdentifierAccess id_list;
  std::vector<std::unique_ptr<Expr>> expr_list;
  std::vector<std::unique_ptr<FuncBlock>> block;

  ForBlock(LineRange pos, IdentifierAccess id_list,
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
    std::vector<std::unique_ptr<FuncBlock>> block;

    IfStatement(std::unique_ptr<Expr> cond,
                std::vector<std::unique_ptr<FuncBlock>> block)
        : cond(std::move(cond)), block(std::move(block)) {}
  };

  std::vector<IfStatement> statements; // series of if { } else if { } ...
  std::optional<std::vector<std::unique_ptr<FuncBlock>>> else_block; // else {}

  IfBlock(LineRange pos, std::vector<IfStatement> statements,
          std::optional<std::vector<std::unique_ptr<FuncBlock>>> else_block)
      : BaseAST(pos), statements(std::move(statements)),
        else_block(std::move(else_block)) {}

  json GetMetaData() const;
};

class TypeExpr : public BaseAST {
 public:
  using GenericId = LineStr;
  struct GenericType {
    using GenericArgs = std::vector<std::variant<std::unique_ptr<TypeExpr>,
                                                 std::unique_ptr<Constant>>>;

    std::unique_ptr<IdentifierAccess> ident;
    // @TODO: currently this only supports literals and type exprs
    //        we probably want to support also constant variables
    //        this is probably something we have to do at semantic stage
    //        since its ambiguous with type exprs

    // @NOTE: could be empty, though I can't of don't want
    // it to be empty just for style reasons since
    // Array[] should really just be Array otherwise it could be misguiding
    // Array[] kinda looks like an array of arrays or Array[Array]
    GenericArgs args;

    GenericType(std::unique_ptr<IdentifierAccess> ident, GenericArgs args)
        : args(std::move(args)), ident(std::move(ident)) {}
  };

  struct VariantType {
    std::unique_ptr<TypeExpr> lhs;
    std::vector<std::unique_ptr<TypeExpr>> rhs;

    VariantType(std::unique_ptr<TypeExpr> lhs,
                std::vector<std::unique_ptr<TypeExpr>> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  };

  struct FunctionType {
    std::optional<LineStr> id;
    std::unique_ptr<TupleDecl> args;
    std::optional<std::unique_ptr<TypeExpr>> ret_type;

    FunctionType(std::optional<LineStr> id, std::unique_ptr<TupleDecl> args,
                 std::optional<std::unique_ptr<TypeExpr>> ret_type)
        : id(id), args(std::move(args)), ret_type(std::move(ret_type)) {}
  };

  std::variant<GenericType, VariantType, IdentifierAccess,
               std::unique_ptr<TupleDecl>, FunctionType, GenericId> expr;

  TypeExpr(LineRange pos, GenericId id)
      : BaseAST(pos), expr(std::move(id)) {}

  TypeExpr(LineRange pos, IdentifierAccess id)
      : BaseAST(pos), expr(std::move(id)) {}

  TypeExpr(LineRange pos, FunctionType func_type)
      : BaseAST(pos), expr(std::move(func_type)) {}

  TypeExpr(LineRange pos, VariantType variant_type)
      : BaseAST(pos), expr(std::move(variant_type)) {}

  TypeExpr(LineRange pos, GenericType generic_type)
      : BaseAST(pos), expr(std::move(generic_type)) {}

  TypeExpr(LineRange pos, std::unique_ptr<TupleDecl> tuple_type)
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