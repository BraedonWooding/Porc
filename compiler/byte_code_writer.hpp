#ifndef BYTE_CODE_WRITER_HPP
#define BYTE_CODE_WRITER_HPP

#include <iostream>
#include <unordered_map>
#include <vector>

#include "ast.hpp"
#include "defs.hpp"
#include "parser.hpp"

namespace interpreter {
extern "C" {
#include <chunk.h>

#include <opcodes.h>
}
}

namespace interpreter {
void WriteChunkData(Chunk *chunk, LineData line, TaggedData data);

template <typename T>
constexpr TaggedData::Type GetType() {
  if constexpr (porc::is_any<T, int64_t>) {
    return TaggedData::INT_LIT;
  } else if constexpr (porc::is_any<T, double>) {
    return TaggedData::FLT_LIT;
  } else if constexpr (porc::is_any<T, char *>) {
    return TaggedData::STR_LIT;
  } else if constexpr (porc::is_any<T, bool>) {
    return TaggedData::BOOL_LIT;
  } else if constexpr (porc::is_any<T, wchar_t>) {
    return TaggedData::CHAR_LIT;
  } else if constexpr (porc::is_any<T, size_t>) {
    return TaggedData::PTR;
  } else if constexpr (porc::is_any<T, u_int8_t>) {
    return TaggedData::REGISTER;
  } else {
    static_assert(porc::always_false<T>::value, "Not a valid opcode type");
  }
}

template <typename T>
constexpr TaggedData::Data GetData(T data) {
  TaggedData::Data output;
  if constexpr (porc::is_any<T, int64_t>) {
    output.int_lit = data;
  } else if constexpr (porc::is_any<T, double>) {
    output.flt_lit = data;
  } else if constexpr (porc::is_any<T, char *>) {
    output.str_lit = data;
  } else if constexpr (porc::is_any<T, bool>) {
    output.bool_lit = data;
  } else if constexpr (porc::is_any<T, wchar_t>) {
    output.char_lit = data;
  } else if constexpr (porc::is_any<T, size_t>) {
    output.ptr = data;
  } else if constexpr (porc::is_any<T, u_int8_t>) {
    output.register_addr = data;
  } else {
    static_assert(porc::always_false<T>::value, "Not a valid opcode type");
  }
  return output;
}

template <typename T1, typename T2, typename T3>
constexpr Instruction NewInstruction(Opcode code, T1 p1, T2 p2, T3 p3) {
  return Instruction{code,
                       {GetType<T1>(), GetType<T2>(), GetType<T3>()},
                       {GetData<T1>(p1), GetData<T2>(p2), GetData<T3>(p3)}};
}

template <typename T1, typename T2>
constexpr Instruction NewInstruction(Opcode code, T1 p1, T2 p2) {
  return Instruction{
      code, {GetType<T1>(), GetType<T2>()}, {GetData<T1>(p1), GetData<T2>(p2)}};
}

template <typename T1>
constexpr Instruction NewInstruction(Opcode code, T1 p1) {
  return Instruction{code, {GetType<T1>()}, {GetData<T1>(p1)}};
}

}  // namespace opcodes

namespace porc {

/*
  Responsible for writing out bytecode to a buffer.
 */
class ByteCodeWriter {
 private:
 public:
  void WriteOpcode(interpreter::Chunk *chunk, interpreter::Instruction instruction,
                   interpreter::LineData line);
};

}  // namespace porc

#endif