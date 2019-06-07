#ifndef HELPER_HPP
#define HELPER_HPP

#include <iostream>

// https://stackoverflow.com/questions/2333728/stdmap-default-value
// too lazy to build my own.  This fits all my purposes.
template <template<class,class,class...> class C, typename K, typename V, typename... Args>
V GetWithDefault(C<K,V,Args...> &m, K const &key, const V &defval)
{
    typename C<K,V,Args...>::const_iterator it = m.find(key);
    if (it == m.end()) {
      m[key] = defval;
      return defval;
    } else {
      return it->second;
    }
}

// taken from boost
template <class T>
inline std::size_t hash_combine(std::size_t seed, const T &v)
{
    std::hash<T> hasher;
    return seed ^ (hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2));
}

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
#define Assert(expr, msg, ...)
#endif

#endif