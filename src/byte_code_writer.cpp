#include "byte_code_writer.hpp"

#include "err_stream.hpp"

#include "byte_code_writer.hpp"
#include "pass_manager.hpp"

namespace porc {

void ByteCodeWriter::WriteOpcode(interpreter::Chunk* chunk,
                                 interpreter::Instruction instruction,
                                 interpreter::LineData line) {
  // write the opcode
  interpreter::write_chunk(chunk, instruction.opcode, line);

  // write all the data segments
  for (int i = 0; i < instruction.data_len; i++) {
    interpreter::WriteChunkData(chunk, line, instruction.data[i]);
  }
}

}  // namespace porc

namespace interpreter {
void WriteChunkData(Chunk* chunk, LineData line, TaggedData data) {
  // output the tagged data type
  write_chunk(chunk, data.type, line);
  switch (data.type) {
    case interpreter::TaggedData::INT_LIT: {
      WRITE_CHUNK_RAW(chunk, data.data.int_lit, line);
    } break;
    case interpreter::TaggedData::FLT_LIT: {
      WRITE_CHUNK_RAW(chunk, data.data.flt_lit, line);
    } break;
    case interpreter::TaggedData::STR_LIT: {
      WRITE_CHUNK_RAW(chunk, data.data.str_lit, line);
    } break;
    case interpreter::TaggedData::BOOL_LIT: {
      WRITE_CHUNK_RAW(chunk, data.data.bool_lit, line);
    } break;
    case interpreter::TaggedData::CHAR_LIT: {
      WRITE_CHUNK_RAW(chunk, data.data.char_lit, line);
    } break;
    case interpreter::TaggedData::PTR: {
      WRITE_CHUNK_RAW(chunk, data.data.ptr, line);
    } break;
    case interpreter::TaggedData::REGISTER: {
      write_chunk_seq(chunk, sizeof(data.data.register_addr),
                      (byte*)&data.data.register_addr, line);
      //WRITE_CHUNK_RAW(chunk, data.data.register_addr, line);
    } break;
    case interpreter::TaggedData::FUNCTION: {
      fprintf(stderr, "TODO");
      abort();
    } break;
    default: {
      fprintf(stderr, "ERROR: Unexpected TaggedData type");
      abort();
    } break;
  }
}
}  // namespace interpreter
