#ifndef DEFS_HPP
#define DEFS_HPP

#include <inttypes.h>
#include <cstddef>
#include <string>

using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i32 = int32_t;
using i64 = int64_t;

using byte = std::byte;

namespace porc {
const int kMajorVersion = 0;
const int kMinorVersion = 1;
const int kSubVersion   = 0;
const std::string kVersion = std::to_string(kMajorVersion) + "." +
                             std::to_string(kMinorVersion) + "." +
                             std::to_string(kSubVersion); 
}

#endif
