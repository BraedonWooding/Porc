# Bytecode in Porc

## Architecture

The Porc interpreter is made up of 3 segments in memory;

- The stack
  - Push and pop functionality
  - Limited size
- Registers (144 + 40 + 100 + 216 = 500 bytes)
  - Each one is 9 bytes base (8 for data 1 for type)
    - The extra byte is for type information
  - The first 16 are for function arguments (each one is 9 bytes)
    - Yes this is where the limit of 16 comes from
    - Note: the limit doesn't apply to var args since those are treated as one together
    - They are referred to as `a0` all the way up to `aF` (hexadecimal)
  - The next 4 are for function return arguments (each one is 9 bytes but there is an extra 4 bytes stored so its a total of 40, extra bytes are useful in tuple representations where you need a length and tuple type)
    - The first (8 + 1) bytes are for shorter function returns (like int) and you can extend it to the others in the case of longer ones (like embedding a complex tuple or a string)
    - They are referred to as `r0, r1, r2, r3`
  - The next 4 * 25 bytes are what we refer to as large registers these are useful for storing strings which are each 24 bytes long inside base (+1 for type)
  - Then there are 24 general purpose addresses which are each 9 bytes

## Instruction header

All instructions present the following header;

```c
| Opcode (8 bits; u8 = 256 opcodes) |
```

## Instructions

### Misc

- `NOP (0)` does explicitly nothing

### Arithmetic

There are the following commands for basic arithmetic;

- `SUB` subtraction
- `MUL` multiplication
- `ADD` addition
- `DIV` division (both args are promoted to floats)
- `INT_DIV` integer division (both are demoted to ints)
- `MOD` modulus
- `POW` power

#### Specialised Arithmetic

Each arithmetic command also has a `REG_` version in which the instruction guarantees that the given memory addresses are 8 bytes.

There are also `INT_` and `FLT_` versions of all instructions denoting that both are integers or both are floating points (respectively) this is implicitly also a `REG_` command.

### Movement

### Functions

### Control Flow
