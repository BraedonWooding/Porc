#ifndef PASS_DATA_HPP
#define PASS_DATA_HPP

#include <iostream>
#include <unordered_map>
#include <vector>
#include <type_traits>

#include "defs.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "pass_manager"

namespace porc {
template<T> void PassManager::SSAPass<T>(std::unique_ptr<T> &expr) {
  // @SAFETY_NET: Hopefully this should catch me doing stupid things
  if constexpr (!std::is_base_of<BaseAST, T>::value) {
    static_assert(always_false<T>::value,
                  "Can't perform a pass on a non AST node");
  }
  return;
}

template<> void PassManager::SSAPass<VarDecl>(
    std::unique_ptr<VarDecl> &expr) {
  Assert(current != nullptr, "must have a valid current scope");

  for (auto &decl : expr->decls) {
    AddVar(decl);
  }
}

template<> void PassManager::SSAPass<TypeDecl>(
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
      err::ReportDualDefinition(msg, (*decl->type)->pos, (*expr->type)->pos,
                               err::SemanticErr,"", "maybe remove this?");
    } else if (!decl->type && expr->type) {
      // we can just move over the type
      decl->type = std::move(expr->type);
      expr->type = std::nullopt;
    }
    // else we don't care about doing anything
  }
}

template<> void PassManager::SSAPass<AssignmentExpr>(
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

}

#endif