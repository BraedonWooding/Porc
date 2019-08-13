#ifndef DEFS_HPP
#define DEFS_HPP

#include <inttypes.h>
#include <cstddef>
#include <string>
#include <optional>
#include <vector>
#include <memory>

// @TODO: Actually figure out a better solution
//        This is just me being lazy
#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#endif

using uint = unsigned int;

using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i32 = int32_t;
using i64 = int64_t;

using byte = unsigned char;
using sbyte = signed char;

// @NOTE: These shouldn't be set
constexpr auto SBYTE_MIN = -128;
constexpr auto SBYTE_MAX = 127;

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
const int kSubVersion = 0;
const std::string kVersion = std::to_string(kMajorVersion) + "." +
std::to_string(kMinorVersion) + "." +
std::to_string(kSubVersion);
// @TEST: make sure to run tests on a smaller buf size since it'll cause
//        a lot more bugs around our token stream.
const size_t TokenizerBufSize = 1024;
const size_t TokenizerMaxLookahead = 2;
}

#endif
