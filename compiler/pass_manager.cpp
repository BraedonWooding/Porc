#include "pass_manager.hpp"

namespace porc {

uint Scope::current_id = 1;

/*
  @NOTE: We could help verify our PerformPass methods by confirming that there
         is ONLY one call to HandleNode; but there has to be atleast ONE
*/

// @TODO: properly template this so we don't get awful errors
//        this is fine for now.
template<typename T, typename V>
void PassManager::AddTo(T map, LineStr id, V *obj) {
  auto it = map.find(id);
  if (it == map.end()) {
    map[id] = obj;
    current->current_ids[id] = 1;
    id.id = 0;
  } else {
    LineStr cpy = id;
    cpy.id = current->current_ids[id]++;
    map[cpy] = obj;
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

template<typename T>
void PassManager::PerformBlockPass(vector_unique_ptr<T> &block) {
  Scope *old = current;
  current = new Scope(Scope::Block, current);
  scopes.push_back(current);
  for (auto &statement: block) PerformPass<T>(statement);
  current = old;
}

template<> void PassManager::PerformPass<IdentifierAccess>(
    std::unique_ptr<IdentifierAccess> &expr) {
  HandleNode(expr);
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

template<>
void PassManager::PerformPass<AssignmentExpr>(
    std::unique_ptr<AssignmentExpr> &expr) {
  HandleNode(expr);

  for (auto &lhs : expr->lhs) PerformPass(lhs);
  for (auto &rhs : expr->rhs) PerformPass(rhs);
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

template<> void PassManager::PerformPass<FuncCall>(
    std::unique_ptr<FuncCall> &expr) {
  HandleNode(expr);
  PerformPass(expr->func);
  for (auto &arg: expr->args) PerformPass(arg);
}

template<> void PassManager::PerformPass<Atom>(
    std::unique_ptr<Atom> &expr) {
  HandleNode(expr);
  std::visit([this](auto &&expr) {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, std::unique_ptr<Expr>, std::unique_ptr<MacroExpr>,
                       std::unique_ptr<Constant>, std::unique_ptr<FuncCall>>) {
      using U = std::decay_t<decltype(*expr)>;
      return PerformPass<U>(expr);
    } else if constexpr (std::is_same_v<T, LineStr>) {
      return; // nothing
    } else if constexpr (std::is_same_v<T, Atom::IndexExpr>) {
      PerformPass<Atom>(expr.lhs);
      PerformPass<Expr>(expr.index);
    } else if constexpr (std::is_same_v<T, Atom::SliceExpr>) {
      PerformPass<Atom>(expr.obj);
      if (expr.start) PerformPass<Expr>(*expr.start);
      if (expr.step) PerformPass<Expr>(*expr.step);
      if (expr.stop) PerformPass<Expr>(*expr.stop);
    } else if constexpr (std::is_same_v<T, Atom::FoldExpr>) {
      PerformPass<Atom>(expr.func);
      PerformPass<AdditiveExpr>(expr.fold_expr);
    } else if constexpr (std::is_same_v<T, Atom::MemberAccessExpr>) {
      PerformPass<Atom>(expr.lhs);
      PerformPass<IdentifierAccess>(expr.access);
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, expr->expr);
}

template<> void PassManager::PerformPass<UnaryExpr>(
    std::unique_ptr<UnaryExpr> &expr) {
  HandleNode(expr);
  PerformPass(expr->rhs);
}

template<> void PassManager::PerformPass<PowerExpr>(
    std::unique_ptr<PowerExpr> &expr) {
  for (auto &expr : expr->exprs) PerformPass(expr);
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

template<> void PassManager::PerformPass<MultiplicativeExpr>(
    std::unique_ptr<MultiplicativeExpr> &expr) {
  HandleNode(expr);
  PerformPass(expr->lhs);
  for (auto &expr : expr->exprs) PerformPass(expr.rhs);
}

template<> void PassManager::PerformPass<AdditiveExpr>(
    std::unique_ptr<AdditiveExpr> &expr) {
  HandleNode(expr);
  PerformPass(expr->lhs);
  for (auto &expr : expr->exprs) PerformPass(expr.rhs);
}

template<> void PassManager::PerformPass<ComparisonExpr>(
    std::unique_ptr<ComparisonExpr> &expr) {
  HandleNode(expr);
  PerformPass(expr->lhs);
  for (auto &expr : expr->exprs) PerformPass(expr.rhs);
}

template<> void PassManager::PerformPass<LogicalAndExpr>(
    std::unique_ptr<LogicalAndExpr> &expr) {
  HandleNode(expr);
  for (auto &expr : expr->exprs) PerformPass(expr);
}

template<> void PassManager::PerformPass<LogicalOrExpr>(
    std::unique_ptr<LogicalOrExpr> &expr) {
  HandleNode(expr);
  for (auto &expr : expr->exprs) PerformPass(expr);
}

template<> void PassManager::PerformPass<VarDecl>(
    std::unique_ptr<VarDecl> &expr) {
  HandleNode(expr);
  for (auto &decl : expr->decls) {
    if (decl.type) PerformPass(*decl.type);
    if (decl.expr) PerformPass(*decl.expr);
  }
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
      PerformBlockPass<FuncStatement>(expr);
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

template<> void PassManager::PerformPass<WhileBlock>(
    std::unique_ptr<WhileBlock> &expr) {
  HandleNode(expr);
  PerformPass(expr->expr);
  PerformBlockPass<FuncStatement>(expr->block);
}

template<> void PassManager::PerformPass<ForBlock>(
    std::unique_ptr<ForBlock> &expr) {
  HandleNode(expr);
  PerformPass(expr->id_list);
  for (auto &expr : expr->expr_list) PerformPass(expr);
  PerformBlockPass<FuncStatement>(expr->block);
}

template<> void PassManager::PerformPass<IfBlock>(
    std::unique_ptr<IfBlock> &expr) {
  HandleNode(expr);
  for (auto &if_statement : expr->statements) {
    PerformPass(if_statement.cond);
    PerformBlockPass<FuncStatement>(if_statement.block);
  }
  if (expr->else_block) PerformBlockPass<FuncStatement>(*expr->else_block);
}

template<> void PassManager::PerformPass<TypeExpr>(
    std::unique_ptr<TypeExpr> &expr) {
  HandleNode(expr);
  std::visit([this](auto &&expr) {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, std::unique_ptr<TupleTypeDecl>,
                            std::unique_ptr<IdentifierAccess>>) {
      using U = std::decay_t<decltype(*expr)>;
      PerformPass<U>(expr);
    } else if constexpr (std::is_same_v<T, TypeExpr::GenericType>) {
      PerformPass<IdentifierAccess>(expr.ident);
      PerformBlockPass<TypeExpr>(expr.args);
    } else if constexpr (std::is_same_v<T, TypeExpr::VariantType>) {
      PerformPass<TypeExpr>(expr.lhs);
      PerformBlockPass<TypeExpr>(expr.rhs);
    } else if constexpr (std::is_same_v<T, TypeExpr::FunctionType>) {
      PerformPass<TupleTypeDecl>(expr.args);
      PerformPass<TypeExpr>(expr.ret_type);
    } else if constexpr (std::is_same_v<T, LineStr>) {
      // nothing
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, expr->expr);
}

template<> void PassManager::PerformPass<Constant>(
    std::unique_ptr<Constant> &expr) {
  HandleNode(expr);
}

template<> void PassManager::PerformPass<MacroExpr>(
    std::unique_ptr<MacroExpr> &expr) {
  HandleNode(expr);
}

template<> void PassManager::PerformPass<TupleValueDecl>(
  std::unique_ptr<TupleValueDecl> &expr) {
  HandleNode(expr);
  for (auto &decl : expr->args) {
    if (decl.type) PerformPass<TypeExpr>(*decl.type);
    if (decl.expr) PerformPass<Expr>(*decl.expr);
  }
}

template<> void PassManager::PerformPass<TupleTypeDecl>(
  std::unique_ptr<TupleTypeDecl> &expr) {
  HandleNode(expr);
  for (auto &decl : expr->args) {
    PerformPass<TypeExpr>(decl.type);
  }
}

}
