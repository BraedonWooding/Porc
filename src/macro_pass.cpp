#ifndef MACRO_PASS_INC
#define MACRO_PASS_INC

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
template<> void PassManager::MacroPass<TypeStatement>(
    std::unique_ptr<TypeStatement> &expr) {
  if (auto macro = std::get_if<std::unique_ptr<MacroExpr>>(&expr->expr)) {
    // we have to mutate the macro
    // @TODO: I'm lazy so I'll probably just chuck a map to funtion pointers
    //        to do this meh.  That's later me problem
  }
}

template<> void PassManager::MacroPass<Atom>(
    std::unique_ptr<Atom> &expr) {
  if (auto macro = std::get_if<std::unique_ptr<MacroExpr>>(&expr->expr)) {
    // we have to mutate the macro
    // @TODO:
  }
}

}

#endif