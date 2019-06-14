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
  std::unordered_map<LineStr, std::unique_ptr<VarDecl>*> initial_decl;
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
class PassManager {
 private:
  // stack like scope vector.
  // @NOTE: we can't just use stack allocated scopes here
  //        else they'll get invalidated on a resize operation.
  std::vector<Scope*> scopes;
  Scope *current = nullptr;

  template<typename T, typename V>
  void AddTo(T map, LineStr id, V *obj);
  void AddVar(VarDecl::Declaration &decl);

  template<typename T>
  bool SSAPass(std::unique_ptr<T> &node); // 100; TRUE

  template<typename T>
  bool HandleNode(std::unique_ptr<T> &node) {
    /*
      @NOTE: adding new passes:
      1) Mark the pass both in the declaration above and in the usage the
         following; priority (positive number, higher is higher priority)
                    mutablility (does it edit the AST)
      2) Confirm that the priority is decreasing and that no 2 mutable passes
         share the same priority.
    */
    if (!SSAPass<T>(node)) return false; // 100; TRUE
    

    return true;
  }

 public:
  PassManager() {}

  template<typename T>
  void PerformPass(std::unique_ptr<T> &node);
};

// template<> void PassManager::PerformPass<>(
//     std::unique_ptr<> &expr);

template<> void PassManager::PerformPass<MacroExpr>(
    std::unique_ptr<MacroExpr> &expr);
template<> void PassManager::PerformPass<Expr>(
    std::unique_ptr<Expr> &expr);
template<> void PassManager::PerformPass<VarDecl>(
    std::unique_ptr<VarDecl> &expr);
template<> void PassManager::PerformPass<TypeStatement>(
    std::unique_ptr<TypeStatement> &expr);
template<> void PassManager::PerformPass<TypeDecl>(
    std::unique_ptr<TypeDecl> &expr);
template<> void PassManager::PerformPass<FuncStatement>(
    std::unique_ptr<FuncStatement> &expr);
template<> void PassManager::PerformPass<AssignmentExpr>(
    std::unique_ptr<AssignmentExpr> &expr);
template<> void PassManager::PerformPass<FileDecl>(
    std::unique_ptr<FileDecl> &expr);

}

#include "ssa_pass.inc"

#endif