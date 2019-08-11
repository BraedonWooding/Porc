#ifndef PASS_DATA_HPP
#define PASS_DATA_HPP

#include <iostream>
#include <unordered_map>
#include <vector>

#include "defs.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "byte_code_writer.hpp"

namespace std {

template <>
struct hash<porc::LineStr>
{
  size_t operator()(const porc::LineStr &str) const
  {
    if (str.id != 0) {
      return hash_combine(str.id, static_cast<string>(str));
    } else {
      return hash<string>()(static_cast<string>(str));
    }
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

  std::unordered_map<LineStr, Expr::FuncDecl*> func_decls;
  std::unordered_map<LineStr, std::unique_ptr<Expr>*> variable_decls;

  template<typename T, typename M>
  std::optional<T> FindInMap(const LineStr &id, const M &map) const {
    auto it = map.find(id);
    if (it == map.end()) {
      return std::nullopt;
    } else {
      return it->second;
    }
  }

  template<typename T>
  std::optional<T> FindLastRec(const LineStr &id) const {
    std::optional<T> val;

    if constexpr (is_any<T, decltype(current_ids)::mapped_type>) {
      val = FindInMap<T>(id, current_ids);
    } else if constexpr (is_any<T, decltype(initial_decl)::mapped_type>) {
      val = FindInMap<T>(id, initial_decl);
    } else if constexpr (is_any<T, decltype(type_decls)::mapped_type>) {
      val = FindInMap<T>(id, type_decls);
    } else if constexpr (is_any<T, decltype(func_decls)::mapped_type>) {
      val = FindInMap<T>(id, func_decls);
    } else if constexpr (is_any<T, decltype(variable_decls)::mapped_type>) {
      val = FindInMap<T>(id, variable_decls);
    } else {
      static_assert(always_false<T>::value, "No matching map for type");
    }

    if (val) {
      return val;
    } else if (parent && *parent) {
        return (*parent)->FindLastRec<T>(id);
    } else {
        return std::nullopt;
    }
  }

  Scope(Kind kind, std::optional<Scope*> parent)
      : id(current_id++), parent(parent), kind(kind) { }
};

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
  bool error_occurred = false;
  Chunk 

  template<typename T>
  void PerformBlockPass(vector_unique_ptr<T> &block);

  template<typename T, typename V>
  void AddTo(T map, LineStr id, V *obj);
  void AddVar(VarDecl::Declaration &decl);

  // \infty; True
  template<typename T>
  void MacroPass(std::unique_ptr<T> &node) {
    // @SAFETY_NET: Hopefully this should catch me doing stupid things
    if constexpr (!std::is_base_of<BaseAST, T>::value) {
      static_assert(always_false<T>::value,
                    "Can't perform a pass on a non AST node");
    }
  }

  // 100; TRUE
  template<typename T>
  void SSAPass(std::unique_ptr<T> &node) {
    // @SAFETY_NET: Hopefully this should catch me doing stupid things
    if constexpr (!std::is_base_of<BaseAST, T>::value) {
      static_assert(always_false<T>::value,
                    "Can't perform a pass on a non AST node");
    }
  }

  // 50; TRUE
  template<typename T>
  void TypePass(std::unique_ptr<T> &node) {
    // @SAFETY_NET: Hopefully this should catch me doing stupid things
    if constexpr (!std::is_base_of<BaseAST, T>::value) {
      static_assert(always_false<T>::value,
                    "Can't perform a pass on a non AST node");
    }
  }

  // 50; TRUE
  template<typename T>
  void ByteCodePass(std::unique_ptr<T> &node) {
    // @SAFETY_NET: Hopefully this should catch me doing stupid things
    if constexpr (!std::is_base_of<BaseAST, T>::value) {
      static_assert(always_false<T>::value,
                    "Can't perform a pass on a non AST node");
    }
  }

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
    if (MacroPass<T>(node), error_occurred) return false; // \infty; True

    if (SSAPass<T>(node), error_occurred) return false; // 100; TRUE

    if (TypePass<T>(node), error_occurred) return false; // 50; TRUE

    if (ByteCodePass<T>(node), error_occurred) return false; // 25; FALSE

    return true;
  }

 public:
  PassManager() {}

  template<typename T>
  void PerformPass(std::unique_ptr<T> &node);
};

template<> void PassManager::PerformPass<IdentifierAccess>(
    std::unique_ptr<IdentifierAccess> &expr);
template<> void PassManager::PerformPass<FileDecl>(
    std::unique_ptr<FileDecl> &expr);
template<> void PassManager::PerformPass<TypeStatement>(
    std::unique_ptr<TypeStatement> &expr);
template<> void PassManager::PerformPass<AssignmentExpr>(
    std::unique_ptr<AssignmentExpr> &expr);
template<> void PassManager::PerformPass<FuncStatement>(
    std::unique_ptr<FuncStatement> &expr);
template<> void PassManager::PerformPass<FuncCall>(
    std::unique_ptr<FuncCall> &expr);
template<> void PassManager::PerformPass<Atom>(
    std::unique_ptr<Atom> &expr);
template<> void PassManager::PerformPass<UnaryExpr>(
    std::unique_ptr<UnaryExpr> &expr);
template<> void PassManager::PerformPass<PowerExpr>(
    std::unique_ptr<PowerExpr> &expr);
template<> void PassManager::PerformPass<TypeDecl>(
    std::unique_ptr<TypeDecl> &expr);
template<> void PassManager::PerformPass<MultiplicativeExpr>(
    std::unique_ptr<MultiplicativeExpr> &expr);
template<> void PassManager::PerformPass<AdditiveExpr>(
    std::unique_ptr<AdditiveExpr> &expr);
template<> void PassManager::PerformPass<ComparisonExpr>(
    std::unique_ptr<ComparisonExpr> &expr);
template<> void PassManager::PerformPass<LogicalAndExpr>(
    std::unique_ptr<LogicalAndExpr> &expr);
template<> void PassManager::PerformPass<LogicalOrExpr>(
    std::unique_ptr<LogicalOrExpr> &expr);
template<> void PassManager::PerformPass<VarDecl>(
    std::unique_ptr<VarDecl> &expr);
template<> void PassManager::PerformPass<Expr>(
    std::unique_ptr<Expr> &expr);
template<> void PassManager::PerformPass<WhileBlock>(
    std::unique_ptr<WhileBlock> &expr);
template<> void PassManager::PerformPass<ForBlock>(
    std::unique_ptr<ForBlock> &expr);
template<> void PassManager::PerformPass<IfBlock>(
    std::unique_ptr<IfBlock> &expr);
template<> void PassManager::PerformPass<TypeExpr>(
    std::unique_ptr<TypeExpr> &expr);
template<> void PassManager::PerformPass<Constant>(
    std::unique_ptr<Constant> &expr);
template<> void PassManager::PerformPass<MacroExpr>(
    std::unique_ptr<MacroExpr> &expr);
template<> void PassManager::PerformPass<TupleValueDecl>(
  std::unique_ptr<TupleValueDecl> &expr);
template<> void PassManager::PerformPass<TupleTypeDecl>(
  std::unique_ptr<TupleTypeDecl> &expr);

}

#endif