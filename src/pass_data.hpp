#ifndef PASS_DATA_HPP
#define PASS_DATA_HPP

#include <iostream>
#include <unordered_map>
#include <vector>

#include "defs.hpp"
#include "parser.hpp"
#include "ast.hpp"

namespace porc {

/*
  A string is SSA form.
  Static single assignment
*/
class StringSSA : public LineStr {
 public:
  uint id; // the associated subscript

  StringSSA(LineStr str, uint id) : id(id), LineStr(str.pos, str) { }
};

}

namespace std {

template <>
struct hash<porc::LineStr>
{
  size_t operator()(const porc::LineStr &str) const
  {
    return hash<string>()(static_cast<string>(str));
  }
};

template <>
struct hash<porc::StringSSA>
{
  size_t operator()(const porc::StringSSA &str) const
  {
    return hash_combine(str.id, static_cast<string>(str));
  }
};

}

namespace porc {

/*
  A given scope.
*/
class Scope {
 private:
  static uint current_id;

 public:
  enum Kind {
    Function,
    Type,
    TopLevel,
    Block
  };

  // you rarely care about your children scopes
  // so we just store the parent scope
  std::optional<Scope*> parent;
  uint id; // id for given scope -- unique
  Kind kind;

  /*
    declarations/definitions
  */

  // used to store the current ssa ids.
  std::unordered_map<LineStr, uint> current_ids;
  std::unordered_map<LineStr, std::unique_ptr<TypeDecl>*> type_decls;

  std::unordered_map<StringSSA, Expr::FuncDecl*> func_decls;
  std::unordered_map<StringSSA, std::unique_ptr<Expr>*> variable_decls;

  Scope(Kind kind, std::optional<Scope*> parent)
      : id(current_id++), parent(parent), kind(kind) { }
};

uint Scope::current_id = 1;

/*
  A collection of useful pass data for passes.
  Collected prior to any passes.
*/
class PassData {
 private:
  // stack like scope vector.
  std::vector<Scope*> scopes;
  ErrStream err;

  Scope *current = nullptr;

 public:
  template<typename T>
  void GatherPassData(std::unique_ptr<T> &top);

  PassData(ErrStream &err) : err(err) {}
};

// template<> void PassData::GatherPassData<>(
//     std::unique_ptr<> &expr);

template<> void PassData::GatherPassData<MacroExpr>(
    std::unique_ptr<MacroExpr> &expr);
template<> void PassData::GatherPassData<Expr>(
    std::unique_ptr<Expr> &expr);
template<> void PassData::GatherPassData<VarDecl>(
    std::unique_ptr<VarDecl> &expr);
template<> void PassData::GatherPassData<TypeStatement>(
    std::unique_ptr<TypeStatement> &expr);
template<> void PassData::GatherPassData<TypeDecl>(
    std::unique_ptr<TypeDecl> &expr);
template<> void PassData::GatherPassData<FuncStatement>(
    std::unique_ptr<FuncStatement> &expr);
template<> void PassData::GatherPassData<AssignmentExpr>(
    std::unique_ptr<AssignmentExpr> &expr);
template<> void PassData::GatherPassData<FileDecl>(
    std::unique_ptr<FileDecl> &expr);

}

#endif