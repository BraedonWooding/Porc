#ifndef DEFS_HPP
#define DEFS_HPP

#include <inttypes.h>
#include <cstddef>
#include <string>
#include <optional>
#include <vector>

using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i32 = int32_t;
using i64 = int64_t;

using byte = std::byte;
using sbyte = signed char;

// @NOTE: These shouldn't be set
#define SBYTE_MIN (-128)
#define SBYTE_MAX (127)

/*
  This just cuts down on the type definitions and gives us valuable horizontal
  space.
*/
template<typename T>
using optional_unique_ptr = std::optional<std::unique_ptr<T>>;

template<typename T>
using vector_unique_ptr = std::vector<std::unique_ptr<T>>;

template<typename T>
using optional_vector_unique_ptr = std::optional<vector_unique_ptr<T>>;

namespace porc {
const int kMajorVersion = 0;
const int kMinorVersion = 1;
const int kSubVersion   = 0;
const std::string kVersion = std::to_string(kMajorVersion) + "." +
                             std::to_string(kMinorVersion) + "." +
                             std::to_string(kSubVersion);
// @TEST: make sure to run tests on a smaller buf size since it'll cause
//        a lot more bugs around our token stream.
const int TokenizerBufSize = 1024;
const int TokenizerMaxLookahead = 2;
}

#endif
