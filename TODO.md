# TODO List

## Front End - Parser

### Tokenizing

- Tokenize chars
- Unicode??

### Parsing

- Implement switches
- Implement pattern matching
- Implement context

## AST

- Add switch, pattern matching, context
- Optimise AST
- Convert type into perhaps an easily comparable form
  - We need something ordinal so we can easily compare

## CodeGen

- Generate some simple bytecode
- Optimisations to bytecode in higher optimisation levels

## Interp

- Write basic interpreter
- Investigate JIT (Low priority)
- Store the last index/member access for a tuple inside the type
  - If there is no 'type' then we can store it generally in the interp
  - This will massively improve the speed if we have for example a for loop going through a tuple.

### Bytecode

- 
