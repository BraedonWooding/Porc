#ifndef BYTE_CODE_PASS_INC
#define BYTE_CODE_PASS_INC

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

/*

  main :: () => {
    return 1 + 2 * (if true 100 else 4);
  }

 */

namespace porc {

template<> inline void PassManager::ByteCodePass<Constant>(std::unique_ptr<Constant> &expr) {

}

}

#endif