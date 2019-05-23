#ifndef HELPER_HPP
#define HELPER_HPP

#include <iostream>

template<typename... Args>
void Assert(bool expr_res, std::string expr_str, std::string msg, uint line,
            std::string file, Args&& ...args) {
  if (!expr_res) {
    std::cerr << file << ":" << line << " Assert failed:\t" << msg << "\n"
              << "Expresssion:\t" << expr_str << "\n";
    if constexpr (sizeof ...(Args) > 0) {
      std::cerr << "Extra Data:\t";
      ((std::cerr << std::forward<Args>(args)), ...);
    }
    std::cerr << std::endl;
    std::abort();
  }
}

#define Unreachable(msg) do {std::cerr << __FILE__ << ":" << __LINE__ \
                                       << " Unreachable:\t" << msg << "\n"; \
                         std::abort(); } while(0)

#ifndef NDEBUG
#define Assert(expr, msg, ...) Assert(expr, #expr, msg, __LINE__, __FILE__, \
                                      ##__VA_ARGS__)
#else
#define Assert(expr, msg, ...) ;
#endif

#endif