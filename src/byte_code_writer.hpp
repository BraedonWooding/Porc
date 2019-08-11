#ifndef BYTE_CODE_WRITER_HPP
#define BYTE_CODE_WRITER_HPP

#include <iostream>
#include <unordered_map>
#include <vector>

#include "defs.hpp"
#include "parser.hpp"
#include "ast.hpp"

namespace chunk {
#include <chunk.h>
}

// scoping it so we don't leak it into our scope
namespace opcodes {

#include <opcodes.h>

template<typename T> constexpr TaggedData::Type GetType() {
  if constexpr (is_any<T, int64_t>::value) {
    return INT_LIT;
  } else if constexpr (is_any<T, double>::value) {
    return FLT_LIT;
  } else if constexpr (is_any<T, char*>::value) {
    return STR_LIT;
  } else if constexpr (is_any<T, bool>::value) {
    return BOOL_LIT;
  } else if constexpr (is_any<T, wchar_t>::value) {
    return CHAR_LIT;
  } else if constexpr (is_any<T, Data*>::value) {
    static_assert(always_false<T>::value,
                  "Shouldn't try to get type of a tuple/array through GetType");
  } else if constexpr (is_any<T, size_t>::value) {
    return PTR;
  } else if constexpr (is_any<T, u_int8_t>::value) {
    return REGISTER;
  } else {
    static_assert(always_false<T>::value, "Not a valid opcode type");
  }
}

template<typename T> constexpr TaggedData::Data GetData(T data) {
	TaggedData::Data data;
  if constexpr (is_any<T, int64_t>::value) {
    data.int_lit = data;
  } else if constexpr (is_any<T, double>::value) {
    data.flt_lit = data;
  } else if constexpr (is_any<T, char*>::value) {
    data.str_lit = data;
  } else if constexpr (is_any<T, bool>::value) {
    data.bool_lit = data;
  } else if constexpr (is_any<T, wchar_t>::value) {
    data.char_lit = data;
  } else if constexpr (is_any<T, Data*>::value) {
    static_assert(always_false<T>::value,
                  "Shouldn't try to get data of a tuple/array through GetData");
  } else if constexpr (is_any<T, size_t>::value) {
    data.ptr = data;
  } else if constexpr (is_any<T, u_int8_t>::value) {
    data.register_addr = data;
  } else {
    static_assert(always_false<T>::value, "Not a valid opcode type");
  }
}

template<typename T1, typename T2, typename T3>
constexpr Instruction NewInstruction(Opcode code, T1 p1, T2 p2, T3 p3) {
  return Instruction() {
    code,
    { GetType<T1>(), GetType<T2>(), GetType<T3>() },
    { GetData<T1>(p1), GetData<T2>(p2), GetData<T3>(p3) }
  };
}

template<typename T1, typename T2>
constexpr Instruction NewInstruction(Opcode code, T1 p1, T2 p2) {
  return Instruction() {
    code,
    { GetType<T1>(), GetType<T2>() },
    { GetData<T1>(p1), GetData<T2>(p2) }
  };
}

template<typename T1>
constexpr Instruction NewInstruction(Opcode code, T1 p1) {
  return Instruction() {
    code,
    { GetType<T1>() },
    { GetData<T1>(p1) }
  };
}

}

namespace porc {

/*
  Responsible for writing out bytecode to a buffer.
 */
class ByteCodeWriter {
private:


public:

  void WriteOpcode(opcodes::Instruction instruction);

private:
};

}

#endif