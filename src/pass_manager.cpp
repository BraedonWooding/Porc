#include "pass_manager.hpp"

namespace porc {

/*
  @NOTE: We could help verify our PerformPass methods by confirming that there
         is ONLY one call to HandleNode; but there has to be atleast ONE
*/

// @TODO: properly template this so we don't get awful errors
//        this is fine for now.
template<typename T, typename V>
void PassManager::AddTo(T map, LineStr id, V *obj) {
  StringSSA first(id, 0);

  auto it = map.find(first);
  if (it == map.end()) {
    map[first] = obj;
    current->current_ids[id] = 1;
  } else {
    StringSSA cur(id, current->current_ids[id]);
    map[cur] = obj;
  }
}

void PassManager::AddVar(VarDecl::Declaration &decl) {
  // NOTE: if no value given then we won't actually place it in the map
  //       @LAZY: Maybe we could get better errors if we actually add it
  if (decl.expr) {
    if (auto func = std::get_if<Expr::FuncDecl>(&(*decl.expr)->expr)) {
      AddTo(current->func_decls, decl.id, func);
    } else {
      AddTo(current->variable_decls, decl.id, &*decl.expr);
    }
  }

  // @TODO: we could also chuck the type in to save time later.
  //        though I kinda would prefer not too
}

template<> void PassManager::PerformPass<MacroExpr>(
    std::unique_ptr<MacroExpr> &expr) {
  HandleNode(expr);
}

template<> void PassManager::PerformPass<Expr>(
    std::unique_ptr<Expr> &expr) {
  HandleNode(expr);
  std::visit([this](auto &&expr)->void {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, std::unique_ptr<VarDecl>, std::unique_ptr<ForBlock>,
                         std::unique_ptr<AssignmentExpr>,
                         std::unique_ptr<WhileBlock>,
                         std::unique_ptr<LogicalOrExpr>,
                         std::unique_ptr<IfBlock>>) {
      using U = std::decay_t<decltype(*expr)>;
      PerformPass<U>(expr);
    } else if constexpr (std::is_same_v<T,
                         std::vector<std::unique_ptr<FuncStatement>>>) {
      Scope *old = current;
      current = new Scope(Scope::Block, current);
      scopes.push_back(current);
      for (auto &statement : expr) {
        PerformPass<FuncStatement>(statement);
      }
      current = old;
    } else if constexpr (std::is_same_v<T, Expr::FuncDecl>) {
      PerformPass(expr.args);
      if (expr.ret_type) PerformPass(*expr.ret_type);

      Scope *old = current;
      current = new Scope(Scope::Function, current);
      scopes.push_back(current);
      for (auto &statement: expr.block) PerformPass(statement);
      current = old;
    } else if constexpr (std::is_same_v<T, Expr::RangeExpr>) {
      if (expr.start) PerformPass(*expr.start);
      if (expr.step) PerformPass(*expr.step);
      if (expr.stop) PerformPass(*expr.stop);
    } else if constexpr (std::is_same_v<T, Expr::CollectionExpr>) {
      for (auto &val: expr.values) PerformPass(val);
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, expr->expr);
}

template<> void PassManager::PerformPass<VarDecl>(
    std::unique_ptr<VarDecl> &expr) {
  HandleNode(expr);
  for (auto &decl : expr->decls) {
    if (decl.type) PerformPass(*decl.type);
    if (decl.expr) PerformPass(*decl.expr);
  }
}

template<> void PassManager::PerformPass<TypeStatement>(
    std::unique_ptr<TypeStatement> &expr) {
  HandleNode(expr);
  std::visit([this](auto &&expr) {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, std::unique_ptr<TypeDecl>,
                         std::unique_ptr<MacroExpr>>) {
      using U = std::decay_t<decltype(*expr)>;
      PerformPass<U>(expr);
    } else if constexpr (is_any<T, TypeStatement::Declaration>) {
      PerformPass<VarDecl>(expr.decl);
      if (expr.access) PerformPass<IdentifierAccess>(*expr.access);
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, expr->expr);
}

template<> void PassManager::PerformPass<TypeDecl>(
    std::unique_ptr<TypeDecl> &expr) {
  HandleNode(expr);
  if (expr->type) PerformPass(*expr->type);

  Scope *old = current;
  current = new Scope(Scope::Type, current);
  scopes.push_back(current);

  for (auto &statement : expr->block) {
    PerformPass<TypeStatement>(statement);
  }

  // restore old
  current = old;
}

template<>
void PassManager::PerformPass<FuncStatement>(
    std::unique_ptr<FuncStatement> &expr) {
  HandleNode(expr);
  std::visit([this](auto &&val) {
    using T = std::decay_t<decltype(*val)>;
    PerformPass<T>(val);
  }, expr->expr);
}

template<>
void PassManager::PerformPass<AssignmentExpr>(
    std::unique_ptr<AssignmentExpr> &expr) {
  HandleNode(expr);

  for (auto &lhs : expr->lhs) PerformPass(lhs);
  for (auto &rhs : expr->rhs) PerformPass(rhs);
}

template<>
void PassManager::PerformPass<FileDecl>(std::unique_ptr<FileDecl> &top) {
  Scope *old = current;
  current = new Scope(Scope::TopLevel, current);
  scopes.push_back(current);

  HandleNode(top);

  for (auto &expr : top->exprs) {
    PerformPass(expr);
  }

  for (auto &type : top->types) {
    PerformPass(type);
  }

  current = old;
}

}
