#ifndef PORC_OPCODES_H
#define PORC_OPCODES_H

/*
  Opcodes are 8 bits
  All addresses ($0x) are 4 byte localized ptrs.
  - The first bit is used to determine the initial point and the other 31 bits
    are used to hold the offset.
  - If first bit is 1 then it is offset from the current stack pointer.
    else it is offset from the register start position.
  NOTE: we only need 31 bits for offset since we hide pointers on the heap
  through passing a ptr like object which is stored on the stack
  all pointers are visited exhaustively for data.
  - I should note that this exhaustive case only occurs in general cases i.e.
    won't occur for INT_ADD or any REG command.
    - While this may seem like we lose optimisation for tuples we don't
      in most cases and we gain a huge optimisation in non tuples.
      - For example if you have a := (1, 2) and you do b :: a[0] + a[1]
        the 'best' avenue is to do a double index (into registers) then an
        INT_ADD into b.
        - We can actually optimise it a bit more by using faster indexers
          such as ARRAY_INDEX_REF which would take the value by reference
          then do a ARRAY_INDEX_NEXT to get the next one (copied into reg)
          then do a MOVE to move the first ref into a reg. then INT_ADD.
          This would probably not be faster for this case.
          But for cases with large tuples since we have to scan linerally
          it could prove to be a huge speed boost to reuse the last one.
          For example a[1000] + a[1001] would be significantly faster.
          For this reason we instead just cache the last call to a struct
          inside the type information and can use this to speed up future calls
          this means that we don't have to optimise it like above we get it for
          free and don't have to pay for the extra MOVE!
      - You may claim that we could realise that they are packed specifically
        and just do something like *(a + some_offset) + *(a + other_offset)
        but that still requires some hidden additions (which we could try to
        absorb into the opcodes but would probably just complicate them and
        require more branches losing more speed then it would gain) the other
        problem is that we can't do this for 'class' tuples since not only
        can they override the index operation but types themselves are
        inconsistent for example an int has a typetag of 1 but a File tuple
        has to have a type tag size of 5 (1 for the type of tuple and 4 for
        the ptr to the File 'type object' - relative) this inconsitency
        would involve branches in the cases of `int | File` for example.
      Basically the exhaustive case applying in general commands is perfectly
      fine and wouldn't speed up other commands by lifting this

  The other two bits are for flags.
  - $0x refers to an address from the current stack frame.
  - $R refers to a register
    - The flag refers to whether or not it goes from current stack frame or
      register.
  - If followed by a `[]` it signifies a type
  - If followed by a `()` it signifies a sizes
    - sb is size_t or native ptr size
  - LIT refers to a literal encoded into the instruction
    - For example LIT just means any type but LIT[str] means a string literal
  - VOID refers to no args
*/

enum Opcode {
  NOP = 0,
  /* Arithmetic */
  // performs it such that c = a + b
  ADD,        // $0x $0x $0x
  SUB,        // $0x $0x $0x
  MUL,        // $0x $0x $0x
  DIV,        // $0x $0x $0x
  // ie integer division
  DIV_FLD,    // $0x $0x $0x
  MOD,        // $0x $0x $0x
  POW,        // $0x $0x $0x

  // wont follow ptrs for REG, INT, or FLT
  REG_ADD,        // $0x $0x $0x
  REG_SUB,        // $0x $0x $0x
  REG_MUL,        // $0x $0x $0x
  REG_DIV,        // $0x $0x $0x
  REG_DIV_FLD,    // $0x $0x $0x
  REG_MOD,        // $0x $0x $0x
  REG_POW,        // $0x $0x $0x

  INT_ADD,        // $0x $0x $0x
  INT_SUB,        // $0x $0x $0x
  INT_MUL,        // $0x $0x $0x
  INT_DIV,        // $0x $0x $0x
  INT_DIV_FLD,    // $0x $0x $0x
  INT_MOD,        // $0x $0x $0x
  INT_POW,        // $0x $0x $0x

  FLT_ADD,        // $0x $0x $0x
  FLT_SUB,        // $0x $0x $0x
  FLT_MUL,        // $0x $0x $0x
  FLT_DIV,        // $0x $0x $0x
  FLT_DIV_FLD,    // $0x $0x $0x
  FLT_MOD,        // $0x $0x $0x
  FLT_POW,        // $0x $0x $0x

  /* Move */
  REG_WRITE_LIT,  // $0x LIT
  WRITE_LIT,      // $0x LIT
  // move between registers
  REG_MOV,    // $0x $0x
  // requires destination object to hold enough space
  MOV,        // $0x $0x
  // pushes object onto stack
  PUSH,       // $0x
  // pushes literal onto stack
  PUSH_LIT,   // LIT
  // decrements the stack ptr by the given amount
  POP,        // LIT[uint](8 bits)
  // sets the stack ptr, absolute ptr
  POP_TO,     // $0x
  // pushes an entire frame onto the stack
  // NOTE: this pointer is absolute (native ptr size)
  PUSH_FRAME, // $0x
  // pops the top frame of the stack
  POP_FRAME,  // VOID
  

  /* Allocation */
  // can't allocate 0 bytes with any of these
  // up to 32 bytes (offset by 1)
  ALLOC_SMALL,  // LIT[uint](5 bits) $0x
  // up to 96 (offset by 33)
  ALLOC_MED,    // LIT[uint](7 bits) $0x
  // > 128 (offset by 129)
  ALLOC_LARGE,  // LIT[usize_t](native ptr) $0x

  // allocate bytes (count from the first address)
  // can allocate 0 bytes (i.e. no allocation occurs)
  ALLOC, // $0x $0x

  // string concatenation (large register format)
  STR_CONCAT,

  // access a member of an object and write copy to output ptr
  // note: won't copy tuples/arrays
  MEMBER_ACCESS,        // $0x LIT[str] $0x
  // always takes ref
  MEMBER_ACCESS_REF,    // $0x LIT[str] $0x
  // write the second ptr value to the member
  MEMBER_ACCESS_WRITE,  // $0x LIT[str] $0x

  // index first object by second and write result into third
  // third has to be a register or stack allocated object
  INDEX,        // $0x $0x $0x
  INDEX_REF,    // $0x $0x $0x
  INDEX_WRITE,  // $0x $0x $0x
};

/*
MEMBER_ACCESS_REF $0x 'c' $0x
b.c = b[i];
b[i] = 2;

REG_WRITE_LIT $R0 0
INDEX $b $R0 $R1
b[0]
*/

#endif