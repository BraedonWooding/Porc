#ifndef PORC_TYPES_H
#define PORC_TYPES_H

#include "porc_common.h"

#ifdef __cplusplus

extern "C" {

#else

typedef enum Opcode Opcode;
typedef struct PorcFunc PorcFunc;
typedef struct Instruction Instruction;
typedef enum PorcType PorcType;
typedef union PorcSize PorcSize;

#endif

enum __PORC_PACKED__ PorcType {
  UNDEFINED = 0b00000000,
  VOID = 0b00000001,

  // u16 rune point
  UNICODE_STR = 0b00000010,
  //
  ASCII_STR = 0b00000011,

  INT8 = 0b00001000,
  INT16 = 0b00001001,
  INT32 = 0b00001010,
  INT64 = 0b00001011,

  UINT8 = 0b00001100,
  UINT16 = 0b00001101,
  UINT32 = 0b00001110,
  UINT64 = 0b00001111,

  BIG_INT = 0b00010000,
  FLT16 = 0b00010001,
  FLT32 = 0b00010010,
  FLT64 = 0b00010011,
  BIG_FLT = 0b00010100,

  // Misc Types
  FUNC = 0b00010101,
  PTR = 0b00010110,

  UNUSED = 0b00010111,

  // Like a dictionary / map but for ASCII_STR -> PTR
  // can be used as a non-library dictionary/map
  // is actually implemented as binary tree map (probably)
  USER_MAP = 0b00011000,

  SIZE_8 = 0b00011001,
  SIZE_16 = 0b00011010,
  SIZE_32 = 0b00011011,
  SIZE_64 = 0b00011100,

  /* Flags */
  GC_TRACKED = 0b10000000,
  // tuples and user data contain a SIZE_X for the size counter
  // arrays instead contain a SIZE_X for the length as well as a
  // byte for a type tag, the type can be set to a SIZE_X
  // to specify the size of each member (not including the corresponding type)
  // to allow for non-homogeneous arrays
  IS_ARRAY = 0b00100000,
  IS_TUPLE = 0b01000000,
  // basically just raw data (except for size counter)
  // completely ignored by program, should use USER_maps to access the data
  USER_DATA = 0b01100000,

  /* Redefinitions for ease */
  ASCII_CHAR = UINT8,
  CHAR = UINT16,
};

struct PorcFunc {
  int n_args;
  size_t address;
};

union PorcSize {
  uint8_t size_8;
  uint16_t size_16;
  uint32_t size_32;
  uint64_t size_64;
};

struct PorcTupleHeader {
  PorcType tuple_type;
  PorcSize count;
};

struct PorcUserDataHeader {
  // should be USER_DATA with a SIZE_X
  PorcType type;
  PorcSize size;
};

struct PorcArrayHeader {
  PorcType array_type;
  PorcType element_type;  // or size
  PorcSize length;
};

/*
  An array that is typed.
 */
union PorcCollectionHeader {
  // same type

  // different types
  struct NonhomogeneousArrayHeader {
    PorcType array_type;
    PorcType element_size;
    PorcSize length;
  } nonhomogeneous_array;
};

#ifdef __cplusplus
}
#endif

#endif