#ifndef SSA_PASS_INC
#define SSA_PASS_INC

// @NOTE: These things shouldn't need to be included
//        since this is a .inc however to make it work nice with IDEs
//        and just general tooling we'll use them here but we should
//        wrap it in a #define that we disable when actually compiling :)
// @TODO: ^^ above.

#include <iostream>
#include <unordered_map>
#include <vector>
#include <type_traits>

#include "defs.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "pass_manager.hpp"

namespace porc {
template<> inline void PassManager::SSAPass<VarDecl>(
    std::unique_ptr<VarDecl> &expr) {
  Assert(current != nullptr, "must have a valid current scope");

  for (auto &decl : expr->decls) {
    AddVar(decl);
  }
}

template<> inline void PassManager::SSAPass<TypeDecl>(
    std::unique_ptr<TypeDecl> &expr) {
  Assert(current != nullptr, "must have a valid current scope");

  auto it = current->FindLastRec<std::unique_ptr<TypeDecl>*>(expr->id);
  if (it) {
    if ((**it)->type && expr->type) {
      const auto msg = "You can't redefine a types definition, if you wish to "
                    "extend it you can just state the name alone with no type";
      err::ReportDualDefinition(msg, (*(**it)->type)->pos, (*expr->type)->pos,
                               err::SemanticErr,"", "maybe remove this?");
    } else if (!(**it)->type && expr->type) {
      (**it)->type = std::move(expr->type);
      expr->type = std::nullopt;
    }
    // else since its already defined we don't redefine
  } else {
    current->type_decls[expr->id] = &expr;
  }
}

template<> inline void PassManager::SSAPass<AssignmentExpr>(
    std::unique_ptr<AssignmentExpr> &expr) {
  Assert(current != nullptr, "must have a valid current scope");

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
      LineStr ident = *static_cast<LineStr*>(data);
      ident.id = 0;
      auto id = current->FindLastRec<uint>(ident);
      if (!id) {
        // no id found so we don't really care but create one current scope
        // @NOTE: We could trigger a flag at this point to indicate a possibly
        //        not declared object (it could exist in some library or smth?)
        current->current_ids[ident] = 0;
        ident.id = 0;
      } else {
        current->current_ids[ident]++;
        ident.id = *id;
      }

      // if its a func decl then put it in the func place else put it in the var
      // it can't be a type
      if (auto func = std::get_if<Expr::FuncDecl>(&(*expr_rhs)->expr)) {
        current->func_decls[ident] = func;
      } else {
        current->variable_decls[ident] = expr_rhs;
      }
    }
  }
}

}

#endif