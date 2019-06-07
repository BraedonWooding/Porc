#include "pass_data.hpp"

namespace porc {

template<>
void PassData::GatherPassData<FuncStatement>(
    std::unique_ptr<FuncStatement> &expr) {
  std::visit([this](auto &&val)->void {
    using T = std::decay_t<decltype(*val)>;
    GatherPassData<T>(val);
  }, expr->expr);
}

template<>
void PassData::GatherPassData<AssignmentExpr>(
    std::unique_ptr<AssignmentExpr> &expr) {
  Assert(current != nullptr, "must have a valid current scope");

  // NOTE: we want to allow SSA
  //       so we want to mark any assignments specially
  for (int i = 0; i < expr->lhs.size(); i++) {
    auto &expr_lhs = expr->lhs[i];
    std::unique_ptr<Expr> *expr_rhs;
    if (expr->rhs.size() == 1) {
      expr_rhs = &expr->rhs[0];
    } else {
      expr_rhs = &expr->rhs[i];
    }

    void *data;
    KindAST type = expr_lhs->UnwrapToLowest(&data);
    if (type == KindAST::Identifier) {
      LineStr *id = static_cast<LineStr*>(data);
      StringSSA ssa(*id, GetWithDefault(current->current_ids, *id, 0u));

      // if its a func decl then put it in the func place else put it in the var
      // it can't be a type
      if (auto func = std::get_if<Expr::FuncDecl>(&(*expr_rhs)->expr)) {
        current->func_decls[ssa] = func;
      } else {
        current->variable_decls[ssa] = expr_rhs;
      }
    }
    // we can't perform SSA on an expr really...?
    // so I guess we just don't????
  }
}

template<>
void PassData::GatherPassData<FileDecl>(std::unique_ptr<FileDecl> &top) {
  current = new Scope(Scope::TopLevel, std::nullopt);
  scopes.push_back(current);

  for (auto &expr : top->exprs) {
    GatherPassData(expr);
  }

  for (auto &type : top->types) {
    GatherPassData(type);
  }
}

}
