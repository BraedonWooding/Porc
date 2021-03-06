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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "defs.hpp"
#include "helper.hpp"
#include "printer_helpers.hpp"
#include "token.hpp"

namespace porc {

template <class T>
struct always_false : std::false_type {};

template <class T, class... Ts>
inline constexpr bool is_any =
    std::bool_constant<(std::is_same_v<T, Ts> || ...)>::value;

enum class KindAST {
  Identifier,
  Double,
  Int,
  String,
  Character,
  Boolean,
  IdentifierAccess,
  FileDecl,
  TypeStatement,
  TypeStatementDeclaration,
  AssignmentExpr,
  FuncStatement,
  FuncCall,
  Atom,
  AtomIndexExpr,
  AtomSliceExpr,
  AtomFoldExpr,
  AtomMemberAccessExpr,
  UnaryExpr,
  PowerExpr,
  TypeDecl,
  MultiplicativeExpr,
  AdditiveExpr,
  ComparisonExpr,
  LogicalAndExpr,
  LogicalOrExpr,
  VarDecl,
  VarDeclDeclaration,
  Expr,
  ExprFuncDecl,
  ExprRangeExpr,
  ExprCollectionExpr,
  ExprBlock,
  WhileBlock,
  ForBlock,
  IfBlock,
  IfBlockStatement,
  TypeExpr,
  TypeExprGeneric,
  TypeExprVariant,
  TypeExprFunction,
  Constant,
  MacroExpr,
  TupleValueDecl,
  TupleTypeDecl,
};

// @TODO: remove this, it was just here so I didn't forget anything
//        either that or make it so that you can't take a reference to it
//        or try todo any polymorphic stuff
class BaseAST {
 protected:
  BaseAST(LineRange pos) : pos(pos) {}

 public:
  LineRange pos;
  BaseAST() = delete;

  // probably want to remove on things that just unwrap themselves...
  virtual KindAST UnwrapToLowest(void **ast) = 0;

  virtual json GetMetaData() const = 0;
};

class IdentifierAccess;
class FileDecl;
class TypeStatement;
class AssignmentExpr;
class FuncStatement;
class FuncCall;
class Atom;
class UnaryExpr;
class PowerExpr;
class TypeDecl;
class MultiplicativeExpr;
class AdditiveExpr;
class ComparisonExpr;
class LogicalAndExpr;
class LogicalOrExpr;
class VarDecl;
class Expr;
class WhileBlock;
class ForBlock;
class IfBlock;
class TypeExpr;
class Constant;
class MacroExpr;
class TupleValueDecl;
class TupleTypeDecl;

std::ostream &operator<<(std::ostream &out,
                         const vector_unique_ptr<FuncStatement> &p);

// @BAD: This is really bad and exposing some nasty stuff
//			 Honestly either change all declarations to using unique
// ptr
// or put this in cpp 		   and put definitions there to hide this away!!
template <typename T>
std::ostream &operator<<(std::ostream &out, const std::unique_ptr<T> &p);

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
    Assign,
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

  friend std::ostream &operator<<(std::ostream &out, const AssignmentOp &p) {
    out << " " << p.ToStr() << " ";
    return out;
  }

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

  friend std::ostream &operator<<(std::ostream &out, const PrefixOp &p) {
    out << " " << p.ToStr() << " ";
    return out;
  }

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

  friend std::ostream &operator<<(std::ostream &out,
                                  const MultiplicativeOp &p) {
    out << " " << p.ToStr() << " ";
    return out;
  }

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

  friend std::ostream &operator<<(std::ostream &out, const AdditiveOp &p) {
    out << " " << p.ToStr() << " ";
    return out;
  }

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

  friend std::ostream &operator<<(std::ostream &out, const ComparisonOp &p) {
    out << " " << p.ToStr() << " ";
    return out;
  }

 private:
  Kind value;
};

class IdentifierAccess : public BaseAST {
 public:
  std::vector<LineStr> idents;

  IdentifierAccess(LineRange pos, std::vector<LineStr> idents)
      : idents(std::move(idents)), BaseAST(pos) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out,
                                  const IdentifierAccess &p) {
    bool first = true;
    for (auto &str : p.idents) {
      if (!first) out << ", ";
      first = false;
      out << str;
    }
    return out;
  }
};

class FileDecl : public BaseAST {
 public:
  vector_unique_ptr<FuncStatement> exprs;
  vector_unique_ptr<TypeDecl> types;

  FileDecl(LineRange pos, vector_unique_ptr<FuncStatement> exprs,
           vector_unique_ptr<TypeDecl> types)
      : exprs(std::move(exprs)), BaseAST(pos), types(std::move(types)) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const FileDecl &p) {
    /* Output the types first then the exprs */
    for (auto &type : p.types) {
      out << type << std::endl;
    }
    for (auto &expr : p.exprs) {
      // @TODO: We could not add a `;` at the end of `}` statements
      //				Or defer the `;` to the func
      // statements??
      out << expr << ";" << std::endl;
    }
    return out;
  }
};

class TypeDecl : public BaseAST {
 public:
  LineStr id;
  optional_unique_ptr<TypeExpr> type;
  vector_unique_ptr<TypeStatement> block;

  TypeDecl(LineRange pos, LineStr id, optional_unique_ptr<TypeExpr> type,
           vector_unique_ptr<TypeStatement> block)
      : BaseAST(pos),
        id(std::move(id)),
        type(std::move(type)),
        block(std::move(block)) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const TypeDecl &p) {
    out << "type " << p.id << " is ";
    if (p.type) out << *p.type;
    if (p.block.size() > 0) {
      out << " {\n";
      for (auto &statement : p.block) {
        // @TODO: same as in FileDecl
        out << statement << ";\n";
      }
      out << "\n}\n";
    } else {
      out << ";\n";
    }

    return out;
  }
};

class VarDecl : public BaseAST {
 public:
  struct Declaration {
    LineStr id;
    optional_unique_ptr<TypeExpr> type;
    optional_unique_ptr<Expr> expr;

    LineRange GetPos() const;

    Declaration(LineStr id, optional_unique_ptr<TypeExpr> type,
                optional_unique_ptr<Expr> expr)
        : id(id), type(std::move(type)), expr(std::move(expr)) {}
  };
  std::vector<Declaration> decls;
  bool is_mut;

  VarDecl(LineRange pos, bool is_mut, std::vector<Declaration> decls)
      : decls(std::move(decls)), is_mut(is_mut), BaseAST(pos) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const VarDecl &p) {
    // @Cleanup, very ugly
    bool has_type = false;
    bool has_value = false;
    bool first = true;

    for (auto &decl : p.decls) {
      if (decl.type) has_type = true;
      if (decl.expr) has_value = true;
      if (!first) out << ", ";
      first = false;
      out << decl.id;
    }

    out << " :";

    if (has_type) {
      first = true;
      for (auto &decl : p.decls) {
        if (!decl.type) continue;
        if (!first) out << ", ";
        first = false;
        out << *decl.type;
      }
    }

    if (has_value) {
      out << (p.is_mut ? "= " : ": ");
      first = true;
      for (auto &decl : p.decls) {
        if (!decl.expr) continue;
        if (!first) out << ", ";
        first = false;
        out << *decl.expr;
      }
    }

    return out;
  }
};

class TypeStatement : public BaseAST {
 public:
  struct Declaration {
    std::unique_ptr<VarDecl> decl;
    optional_unique_ptr<IdentifierAccess> access;

    Declaration(std::unique_ptr<VarDecl> decl,
                optional_unique_ptr<IdentifierAccess> access)
        : decl(std::move(decl)), access(std::move(access)) {}
  };

  std::variant<std::unique_ptr<TypeDecl>, Declaration,
               std::unique_ptr<MacroExpr>>
      expr;

  TypeStatement(LineRange pos, std::unique_ptr<TypeDecl> expr)
      : BaseAST(pos), expr(std::move(expr)) {}

  TypeStatement(LineRange pos, std::unique_ptr<VarDecl> expr,
                optional_unique_ptr<IdentifierAccess> id)
      : BaseAST(pos), expr(Declaration(std::move(expr), std::move(id))) {}

  TypeStatement(LineRange pos, std::unique_ptr<MacroExpr> expr)
      : BaseAST(pos), expr(std::move(expr)) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const TypeStatement &p);
};

class FuncStatement : public BaseAST {
 public:
  std::variant<std::unique_ptr<AssignmentExpr>, std::unique_ptr<Expr>,
               std::unique_ptr<VarDecl>>
      expr;

  enum PrefixKind : u8 {
    NoPrefix = 0b00000000,
    Yield = 0b00000001,   // reserving the first bit for yield
    Return = 0b00000010,  // the rest fully ignore the first bit
    Continue = 0b00000100,
    Break = 0b00000110,
    BlockVal = 0b00001000,  // i.e. { = 4; }
  };

  PrefixKind prefix;

  bool HasPrefix(PrefixKind kind) const { return (prefix & kind) != 0; }

  FuncStatement(LineRange pos, std::unique_ptr<AssignmentExpr> expr)
      : BaseAST(pos), expr(std::move(expr)), prefix(NoPrefix) {}

  FuncStatement(LineRange pos, std::unique_ptr<Expr> expr, PrefixKind kind)
      : BaseAST(pos), expr(std::move(expr)), prefix(kind) {}

  FuncStatement(LineRange pos, std::unique_ptr<VarDecl> expr)
      : BaseAST(pos), expr(std::move(expr)), prefix(NoPrefix) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const FuncStatement &p);
};

// NOTE: This has a weird ish parsing logic if you don't do `id ':' type_expr`
//       then it automatically is just a type expr which means it doesn't
//       follow the standard of a function declaration
// i.e. (a, b) defines it like (id1: a, id2: b) rather than (a: $A, b: $B)
//      this may confuse people and we may want to change this
//      however it currently is only used in type exprs in which case it
//      makes more sense since if you are specifying a type you should fully
//      specify it however you don't have to fully specify generics
// i.e. (Map[$A, $B]) is fine and so is (Map) soooooo is this very odd?
//      I don't know how I would want it changed since I like the ability
//      to have 'C' like types i.e. (int, flt)->str since it makes it simpler
//      to type them.  Maybe we just focus on giving better errors
// @FIX @GLARING_ISSUE @ISSUE @EBNF
class TupleTypeDecl : public BaseAST {
 public:
  struct ArgDecl {
    std::unique_ptr<TypeExpr> type;
    std::optional<LineStr> id;

    ArgDecl(std::optional<LineStr> id, std::unique_ptr<TypeExpr> type)
        : id(id), type(std::move(type)) {}
  };

  std::vector<ArgDecl> args;

  TupleTypeDecl(LineRange pos, std::vector<ArgDecl> args)
      : args(std::move(args)), BaseAST(pos) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const TupleTypeDecl &p) {
    bool first = true;
    out << "(";
    for (auto &arg : p.args) {
      if (!first) out << ", ";
      first = false;

      if (arg.id) out << *arg.id << " : ";
      out << arg.type;
    }
    out << ")";
    return out;
  }
};

class TupleValueDecl : public BaseAST {
 public:
  struct ArgDecl {
    optional_unique_ptr<TypeExpr> type;
    LineStr id;
    optional_unique_ptr<Expr> expr;

    ArgDecl(LineStr id, optional_unique_ptr<TypeExpr> type,
            optional_unique_ptr<Expr> expr)
        : id(id), type(std::move(type)), expr(std::move(expr)) {}
  };

  std::vector<ArgDecl> args;

  TupleValueDecl(LineRange pos, std::vector<ArgDecl> args)
      : args(std::move(args)), BaseAST(pos) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const TupleValueDecl &p) {
    bool first = true;
    out << "(";
    for (auto &arg : p.args) {
      if (!first) out << ", ";
      first = false;

      out << arg.id;
      if (arg.type) {
        out << " : " << *arg.type;
      }
      if (arg.expr) {
        out << " = " << *arg.expr;
      }
    }
    out << ")";
    return out;
  }
};

class MacroExpr : public BaseAST {
 public:
  std::unique_ptr<IdentifierAccess> qualifying_name;
  vector_unique_ptr<Expr> args;

  MacroExpr(LineRange pos, std::unique_ptr<IdentifierAccess> qualifying_name,
            vector_unique_ptr<Expr> args)
      : BaseAST(pos),
        qualifying_name(std::move(qualifying_name)),
        args(std::move(args)) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const MacroExpr &p) {
    out << "@" << p.qualifying_name << "(";
    bool first = true;
    for (auto &arg : p.args) {
      if (!first) out << ", ";
      first = false;
      out << arg;
    }
    out << ")";
    return out;
  }
};

class AssignmentExpr : public BaseAST {
 public:
  vector_unique_ptr<Expr> lhs;
  AssignmentOp op;
  vector_unique_ptr<Expr> rhs;

  AssignmentExpr(LineRange pos, vector_unique_ptr<Expr> lhs, AssignmentOp op,
                 vector_unique_ptr<Expr> rhs)
      : BaseAST(pos), lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const AssignmentExpr &p) {
    bool first = true;
    for (auto &arg : p.lhs) {
      if (!first) out << ", ";
      first = false;
      out << arg;
    }

    out << p.op;

    first = true;
    for (auto &arg : p.rhs) {
      if (!first) out << ", ";
      first = false;
      out << arg;
    }
    return out;
  }
};

class FuncCall : public BaseAST {
 public:
  std::unique_ptr<Atom> func;
  vector_unique_ptr<Expr> args;

  FuncCall(LineRange pos, std::unique_ptr<Atom> expr,
           vector_unique_ptr<Expr> args)
      : BaseAST(pos), func(std::move(expr)), args(std::move(args)) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const FuncCall &p) {
    out << p.func << "(";

    bool first = true;
    for (auto &arg : p.args) {
      if (!first) out << ", ";
      first = false;
      out << arg;
    }
    out << ")";
    return out;
  }
};

class Atom : public BaseAST {
 public:
  struct IndexExpr {
    std::unique_ptr<Atom> lhs;
    std::unique_ptr<Expr> index;

    IndexExpr(std::unique_ptr<Atom> lhs, std::unique_ptr<Expr> index)
        : lhs(std::move(lhs)), index(std::move(index)) {}
  };

  struct SliceExpr {
    std::unique_ptr<Atom> obj;
    optional_unique_ptr<Expr> start;
    optional_unique_ptr<Expr> stop;
    optional_unique_ptr<Expr> step;

    SliceExpr(std::unique_ptr<Atom> obj, optional_unique_ptr<Expr> start,
              optional_unique_ptr<Expr> stop, optional_unique_ptr<Expr> step)
        : obj(std::move(obj)),
          start(std::move(start)),
          stop(std::move(stop)),
          step(std::move(step)) {}
  };

  struct FoldExpr {
    std::unique_ptr<Atom> func;
    std::unique_ptr<AdditiveExpr> fold_expr;
    bool folds_right;  // |> vs <|

    FoldExpr(std::unique_ptr<Atom> func, bool folds_right,
             std::unique_ptr<AdditiveExpr> fold_expr)
        : func(std::move(func)),
          fold_expr(std::move(fold_expr)),
          folds_right(folds_right) {}
  };

  struct MemberAccessExpr {
    std::unique_ptr<Atom> lhs;
    std::unique_ptr<IdentifierAccess> access;

    MemberAccessExpr(std::unique_ptr<Atom> lhs,
                     std::unique_ptr<IdentifierAccess> access)
        : lhs(std::move(lhs)), access(std::move(access)) {}
  };

  std::variant<IndexExpr, SliceExpr, std::unique_ptr<FuncCall>, FoldExpr,
               std::unique_ptr<Expr>, MemberAccessExpr,
               std::unique_ptr<MacroExpr>, std::unique_ptr<Constant>, LineStr>
      expr;

  Atom(LineRange pos, LineStr id) : BaseAST(pos), expr(id) {}

  Atom(LineRange pos, std::unique_ptr<Expr> expr)
      : BaseAST(pos), expr(std::move(expr)) {}

  Atom(LineRange pos, std::unique_ptr<Atom> lhs, std::unique_ptr<Expr> index)
      : BaseAST(pos), expr(IndexExpr(std::move(lhs), std::move(index))) {}

  Atom(LineRange pos, std::unique_ptr<Atom> lhs,
       optional_unique_ptr<Expr> start, optional_unique_ptr<Expr> stop,
       optional_unique_ptr<Expr> step)
      : BaseAST(pos),
        expr(SliceExpr(std::move(lhs), std::move(start), std::move(stop),
                       std::move(step))) {}

  Atom(LineRange pos, std::unique_ptr<FuncCall> func_call)
      : BaseAST(pos), expr(std::move(func_call)) {}

  Atom(LineRange pos, std::unique_ptr<Atom> func, bool folds_right,
       std::unique_ptr<AdditiveExpr> rhs)
      : BaseAST(pos),
        expr(FoldExpr(std::move(func), folds_right, std::move(rhs))) {}

  Atom(LineRange pos, std::unique_ptr<Atom> lhs,
       std::unique_ptr<IdentifierAccess> access)
      : BaseAST(pos),
        expr(MemberAccessExpr(std::move(lhs), std::move(access))) {}

  Atom(LineRange pos, std::unique_ptr<MacroExpr> expr)
      : BaseAST(pos), expr(std::move(expr)) {}

  Atom(LineRange pos, std::unique_ptr<Constant> expr)
      : BaseAST(pos), expr(std::move(expr)) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const Atom &p) {
    std::visit(
        [&out, &p](auto &&expr) {
          using T = std::decay_t<decltype(expr)>;
          if constexpr (is_any<T, std::unique_ptr<Expr>,
                               std::unique_ptr<MacroExpr>,
                               std::unique_ptr<Constant>,
                               std::unique_ptr<FuncCall>, LineStr>) {
            out << expr;
          } else if constexpr (std::is_same_v<T, Atom::IndexExpr>) {
            out << expr.lhs << "[" << expr.index << "]";
          } else if constexpr (is_any<T, Atom::SliceExpr>) {
            out << expr.obj << "[";
            if (expr.start) out << *expr.start;
            out << ":";
            if (expr.stop) out << *expr.stop;
            out << ":";
            if (expr.step) out << *expr.step;
          } else if constexpr (std::is_same_v<T, Atom::FoldExpr>) {
            if (expr.folds_right) {
              out << expr.fold_expr << " |> " << expr.func;
            } else {
              out << expr.func << " <| " << expr.fold_expr;
            }
          } else if constexpr (std::is_same_v<T, Atom::MemberAccessExpr>) {
            out << expr.lhs << "." << expr.access;
          } else {
            static_assert(always_false<T>::value, "non-exhaustive vistor!");
          }
        },
        p.expr);
    return out;
  }
};

class PowerExpr : public BaseAST {
 public:
  vector_unique_ptr<Atom> exprs;

  PowerExpr(LineRange pos, std::unique_ptr<Atom> expr) : BaseAST(pos) {
    exprs.push_back(std::move(expr));
  }

  PowerExpr(LineRange pos, vector_unique_ptr<Atom> exprs)
      : exprs(std::move(exprs)), BaseAST(pos) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const PowerExpr &p) {
    bool first = true;
    for (auto &arg : p.exprs) {
      if (!first) out << " ** ";
      first = false;
      out << arg;
    }
    return out;
  }
};

class UnaryExpr : public BaseAST {
 public:
  std::unique_ptr<PowerExpr> rhs;
  std::vector<PrefixOp> ops;

  UnaryExpr(LineRange pos, std::unique_ptr<PowerExpr> expr)
      : BaseAST(pos), rhs(std::move(expr)) {}

  UnaryExpr(LineRange pos, std::unique_ptr<PowerExpr> expr,
            std::vector<PrefixOp> ops)
      : BaseAST(pos), rhs(std::move(expr)), ops(ops) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const UnaryExpr &p) {
    for (auto &arg : p.ops) out << arg;
    out << p.rhs;
    return out;
  }
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
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out,
                                  const MultiplicativeExpr &p) {
    out << p.lhs;
    for (auto &arg : p.exprs) out << arg.op << arg.rhs;
    return out;
  }
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
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const AdditiveExpr &p) {
    out << p.lhs;
    for (auto &arg : p.exprs) out << arg.op << arg.rhs;
    return out;
  }
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
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const ComparisonExpr &p) {
    out << p.lhs;
    for (auto &arg : p.exprs) out << arg.op << arg.rhs;
    return out;
  }
};

class LogicalAndExpr : public BaseAST {
 public:
  vector_unique_ptr<ComparisonExpr> exprs;

  LogicalAndExpr(LineRange pos, std::unique_ptr<ComparisonExpr> expr)
      : BaseAST(pos) {
    exprs.push_back(std::move(expr));
  }

  LogicalAndExpr(LineRange pos, vector_unique_ptr<ComparisonExpr> exprs)
      : exprs(std::move(exprs)), BaseAST(pos) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const LogicalAndExpr &p) {
    bool first = true;
    for (auto &arg : p.exprs) {
      if (!first) out << " && ";
      first = false;
      out << arg;
    }
    return out;
  }
};

class LogicalOrExpr : public BaseAST {
 public:
  vector_unique_ptr<LogicalAndExpr> exprs;

  LogicalOrExpr(LineRange pos, std::unique_ptr<LogicalAndExpr> expr)
      : BaseAST(pos) {
    exprs.push_back(std::move(expr));
  }

  LogicalOrExpr(LineRange pos, vector_unique_ptr<LogicalAndExpr> exprs)
      : exprs(std::move(exprs)), BaseAST(pos) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const LogicalOrExpr &p) {
    bool first = true;
    for (auto &arg : p.exprs) {
      if (!first) out << " || ";
      first = false;
      out << arg;
    }
    return out;
  }
};

class Expr : public BaseAST {
 public:
  struct FuncDecl {
    std::unique_ptr<TupleValueDecl> args;
    optional_unique_ptr<TypeExpr> ret_type;
    vector_unique_ptr<FuncStatement> block;

    FuncDecl(std::unique_ptr<TupleValueDecl> args,
             optional_unique_ptr<TypeExpr> ret_type,
             vector_unique_ptr<FuncStatement> block)
        : args(std::move(args)),
          block(std::move(block)),
          ret_type(std::move(ret_type)) {}
  };

  struct RangeExpr {
    optional_unique_ptr<LogicalOrExpr> start;
    optional_unique_ptr<LogicalOrExpr> stop;
    optional_unique_ptr<LogicalOrExpr> step;
    bool inclusive;

    RangeExpr(optional_unique_ptr<LogicalOrExpr> start,
              optional_unique_ptr<LogicalOrExpr> stop,
              optional_unique_ptr<LogicalOrExpr> step, bool inclusive)
        : start(std::move(start)),
          stop(std::move(stop)),
          step(std::move(step)),
          inclusive(inclusive) {}
  };

  struct CollectionExpr {
   private:
    bool collection_is_array;

   public:
    vector_unique_ptr<Expr> values;
    bool IsArray() const { return this->collection_is_array; };
    bool IsTuple() const { return !this->collection_is_array; }

    CollectionExpr(vector_unique_ptr<Expr> values, bool is_array)
        : values(std::move(values)), collection_is_array(is_array) {}
  };

  std::variant<std::unique_ptr<LogicalOrExpr>, FuncDecl,
               std::unique_ptr<VarDecl>, RangeExpr, std::unique_ptr<ForBlock>,
               std::unique_ptr<WhileBlock>, std::unique_ptr<IfBlock>,
               CollectionExpr, std::unique_ptr<AssignmentExpr>,
               vector_unique_ptr<FuncStatement>>
      expr;

  Expr(LineRange pos, std::unique_ptr<LogicalOrExpr> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, std::unique_ptr<VarDecl> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, std::unique_ptr<AssignmentExpr> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, bool inclusive, optional_unique_ptr<LogicalOrExpr> start,
       optional_unique_ptr<LogicalOrExpr> stop,
       optional_unique_ptr<LogicalOrExpr> step)
      : BaseAST(pos),
        expr(RangeExpr(std::move(start), std::move(stop), std::move(step),
                       inclusive)) {}
  Expr(LineRange pos, std::unique_ptr<TupleValueDecl> args,
       optional_unique_ptr<TypeExpr> ret_type,
       vector_unique_ptr<FuncStatement> block)
      : BaseAST(pos),
        expr(FuncDecl(std::move(args), std::move(ret_type), std::move(block))) {
  }
  Expr(LineRange pos, vector_unique_ptr<Expr> members, bool is_array)
      : BaseAST(pos), expr(CollectionExpr(std::move(members), is_array)) {}
  Expr(LineRange pos, std::unique_ptr<IfBlock> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, std::unique_ptr<WhileBlock> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, std::unique_ptr<ForBlock> arg)
      : expr(std::move(arg)), BaseAST(pos) {}
  Expr(LineRange pos, vector_unique_ptr<FuncStatement> block)
      : expr(std::move(block)), BaseAST(pos) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const Expr &p) {
    std::visit(
        [&out, &p](auto &&expr) {
          using T = std::decay_t<decltype(expr)>;
          if constexpr (is_any<T, std::unique_ptr<AssignmentExpr>,
                               std::unique_ptr<VarDecl>>) {
            out << "let " << expr;
          } else if constexpr (is_any<T, std::unique_ptr<LogicalOrExpr>,
                                      std::unique_ptr<WhileBlock>,
                                      std::unique_ptr<ForBlock>,
                                      std::unique_ptr<IfBlock>>) {
            out << expr;
          } else if constexpr (std::is_same_v<T, std::vector<std::unique_ptr<
                                                     FuncStatement>>>) {
            bool first = true;
            for (auto &arg : expr) {
              if (!first) out << ", ";
              first = false;
              out << arg;
            }
          } else if constexpr (std::is_same_v<T, Expr::FuncDecl>) {
            out << expr.args;
            if (expr.ret_type) out << "->" << *expr.ret_type;
            out << " => ";
            out << expr.block;
          } else if constexpr (std::is_same_v<T, Expr::RangeExpr>) {
            if (expr.start) out << *expr.start;
            out << "..";
            if (expr.inclusive) out << "=";
            if (expr.stop) out << *expr.stop;
            if (expr.step) out << ":" << *expr.step;
          } else if constexpr (std::is_same_v<T, Expr::CollectionExpr>) {
            out << (expr.IsArray() ? "[" : "(");
            bool first = true;
            for (auto &arg : expr.values) {
              if (!first) out << ", ";
              first = false;
              out << arg;
            }
            out << (expr.IsArray() ? "]" : ")");
          } else {
            static_assert(always_false<T>::value, "non-exhaustive vistor!");
          }
        },
        p.expr);
    return out;
  }
};

class WhileBlock : public BaseAST {
 public:
  std::unique_ptr<Expr> expr;
  vector_unique_ptr<FuncStatement> block;

  WhileBlock(LineRange pos, std::unique_ptr<Expr> expr,
             vector_unique_ptr<FuncStatement> block)
      : BaseAST(pos), expr(std::move(expr)), block(std::move(block)) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const WhileBlock &p) {
    out << "while (" << p.expr << ") " << p.block;
    return out;
  }
};

class ForBlock : public BaseAST {
 public:
  std::unique_ptr<IdentifierAccess> id_list;
  vector_unique_ptr<Expr> expr_list;
  vector_unique_ptr<FuncStatement> block;

  ForBlock(LineRange pos, std::unique_ptr<IdentifierAccess> id_list,
           vector_unique_ptr<Expr> expr_list,
           vector_unique_ptr<FuncStatement> block)
      : BaseAST(pos),
        id_list(std::move(id_list)),
        expr_list(std::move(expr_list)),
        block(std::move(block)) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const ForBlock &p) {
    out << "for (" << p.id_list << " in ";

    bool first = true;
    for (auto &arg : p.expr_list) {
      if (!first) out << ", ";
      first = false;
      out << arg;
    }

    out << ") ";
    out << p.block;
    return out;
  }
};

class IfBlock : public BaseAST {
 public:
  struct IfStatement {
    std::unique_ptr<Expr> cond;
    vector_unique_ptr<FuncStatement> block;

    IfStatement(std::unique_ptr<Expr> cond,
                vector_unique_ptr<FuncStatement> block)
        : cond(std::move(cond)), block(std::move(block)) {}
  };

  std::vector<IfStatement> statements;  // series of if { } else if { } ...
  optional_vector_unique_ptr<FuncStatement> else_block;  // else {}

  IfBlock(LineRange pos, std::vector<IfStatement> statements,
          optional_vector_unique_ptr<FuncStatement> else_block)
      : BaseAST(pos),
        statements(std::move(statements)),
        else_block(std::move(else_block)) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const IfBlock &p) {
    bool first = true;
    for (auto &statement : p.statements) {
      if (!first) {
        out << "else ";
      }
      first = false;
      out << "if (" << statement.cond << ") " << statement.block;
    }
    if (p.else_block) out << *p.else_block;
    return out;
  }
};

class TypeExpr : public BaseAST {
 public:
  struct GenericType {
    std::unique_ptr<IdentifierAccess> ident;
    vector_unique_ptr<TypeExpr> args;

    GenericType(std::unique_ptr<IdentifierAccess> ident,
                vector_unique_ptr<TypeExpr> args)
        : args(std::move(args)), ident(std::move(ident)) {}
  };

  struct VariantType {
    std::unique_ptr<TypeExpr> lhs;
    vector_unique_ptr<TypeExpr> rhs;

    VariantType(std::unique_ptr<TypeExpr> lhs, vector_unique_ptr<TypeExpr> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  };

  struct FunctionType {
    std::unique_ptr<TupleTypeDecl> args;
    std::unique_ptr<TypeExpr> ret_type;

    FunctionType(std::unique_ptr<TupleTypeDecl> args,
                 std::unique_ptr<TypeExpr> ret_type)
        : args(std::move(args)), ret_type(std::move(ret_type)) {}
  };

  std::variant<GenericType, VariantType, std::unique_ptr<IdentifierAccess>,
               std::unique_ptr<TupleTypeDecl>, FunctionType, LineStr>
      expr;

  TypeExpr(LineRange pos, LineStr id) : BaseAST(pos), expr(std::move(id)) {}

  TypeExpr(LineRange pos, std::unique_ptr<IdentifierAccess> id)
      : BaseAST(pos), expr(std::move(id)) {}

  TypeExpr(LineRange pos, FunctionType func_type)
      : BaseAST(pos), expr(std::move(func_type)) {}

  TypeExpr(LineRange pos, VariantType variant_type)
      : BaseAST(pos), expr(std::move(variant_type)) {}

  TypeExpr(LineRange pos, GenericType generic_type)
      : BaseAST(pos), expr(std::move(generic_type)) {}

  TypeExpr(LineRange pos, std::unique_ptr<TupleTypeDecl> tuple_type)
      : BaseAST(pos), expr(std::move(tuple_type)) {}

  json GetMetaData() const;
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const TypeExpr &p) {
    std::visit(
        [&out, &p](auto &&expr) {
          using T = std::decay_t<decltype(expr)>;
          if constexpr (is_any<T, std::unique_ptr<TupleTypeDecl>,
                               std::unique_ptr<IdentifierAccess>, LineStr>) {
            out << expr;
          } else if constexpr (std::is_same_v<T, TypeExpr::GenericType>) {
            out << expr.ident;
            if (expr.args.size() > 0) {
              out << "[";
              bool first = true;
              for (auto &expr : expr.args) {
                if (!first) out << ", ";
                first = false;
                out << expr;
              }
              out << "]";
            }
          } else if constexpr (std::is_same_v<T, TypeExpr::VariantType>) {
            out << expr.lhs;
            bool first = true;
            for (auto &expr : expr.rhs) {
              if (!first) out << " | ";
              first = false;
              out << expr;
            }
          } else if constexpr (std::is_same_v<T, TypeExpr::FunctionType>) {
            out << expr.args << "->" << expr.ret_type;
          } else {
            static_assert(always_false<T>::value, "non-exhaustive vistor!");
          }
        },
        p.expr);
    return out;
  }
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
  KindAST UnwrapToLowest(void **ast);

  friend std::ostream &operator<<(std::ostream &out, const Constant &p) {
    std::visit(
        [&out, &p](auto &&expr) {
          using T = std::decay_t<decltype(expr)>;
          if constexpr (std::is_same_v<T, double>) {
            out << expr;
          } else if constexpr (std::is_same_v<T, i64>) {
            out << expr;
          } else if constexpr (std::is_same_v<T, std::string>) {
            out << '"' << expr << '"';
          } else if constexpr (std::is_same_v<T, char>) {
            out << "'" << expr << "'";
          } else if constexpr (std::is_same_v<T, bool>) {
            out << (expr ? "true" : "false");
          } else {
            static_assert(always_false<T>::value, "non-exhaustive vistor!");
          }
        },
        p.data);
    return out;
  }
};

}  // namespace porc

#endif