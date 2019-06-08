#include "pass_data.hpp"



namespace porc {

template<> void PassData::GatherPassData<MacroExpr>(
    std::unique_ptr<MacroExpr> &expr) {
  // @TODO
}

template<> void PassData::GatherPassData<Expr>(
    std::unique_ptr<Expr> &expr) {
    
}

template<> void PassData::GatherPassData<VarDecl>(
    std::unique_ptr<VarDecl> &expr) {

}

template<> void PassData::GatherPassData<TypeStatement>(
    std::unique_ptr<TypeStatement> &expr) {
  std::visit([this](auto &&expr) {
    using T = std::decay_t<decltype(expr)>;
    if constexpr (is_any<T, std::unique_ptr<TypeDecl>,
                            std::unique_ptr<MacroExpr>>) {
      using U = std::decay_t<decltype(*expr)>;
      GatherPassData<U>(expr);
    } else if constexpr (std::is_same_v<T, TypeStatement::Declaration>) {
      return;
    } else {
      static_assert(always_false<T>::value, "non-exhaustive vistor!");
    }
  }, expr->expr);
}

template<> void PassData::GatherPassData<TypeDecl>(
    std::unique_ptr<TypeDecl> &expr) {
  Assert(current != nullptr, "must have a valid current scope");

  auto it = current->type_decls.find(expr->id);
  if (it == current->type_decls.end()) {
    current->type_decls[expr->id] = &expr;
  } else {
    Assert(it->second != nullptr, "can't have null type decls inside map");
    auto &decl = *it->second;
    if (decl->type && expr->type) {
      const auto msg = "You can't redefine a types definition, if you wish to "
                    "extend it you can just state the name alone with no type";
      err.ReportDualDefinition(msg, (*decl->type)->pos, (*expr->type)->pos,
                               ErrStream::SemanticErr,"", "maybe remove this?");
    } else if (!decl->type && expr->type) {
      // we can just move over the type
      decl->type = std::move(expr->type);
      expr->type = std::nullopt;
    }
    // else we don't care about doing anything
  }

  for (auto &statement : expr->block) {
    GatherPassData<TypeStatement>(statement);
  }
}

template<>
void PassData::GatherPassData<FuncStatement>(
    std::unique_ptr<FuncStatement> &expr) {
  std::visit([this](auto &&val) {
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
    // the other argument is that we also support non string 'lhs'
    // the argument however is that those aren't variables so we
    // shouldn't factor them out...?
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
