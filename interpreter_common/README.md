# Backend Porc Interpreter (or Bacon) Infrastructure

> This is intended for VM maintainers and for those who wish to target porc bytecode.

Porc's interpreter is relatively simple but there are a few places where stuff is done unintuitively in the interest for efficiency or some other benefit.

## Chunks

Code is stored in chunks of instructions, these instructions are jagged and aren't sized consistently.  There are a ridiculous amount of instructions that we implement in our core loop using a computed goto (from what I've seen it is around ~25% faster than a standard switch).

Each byte in the chunk has a corresponding LineData.  This is currently just the line but it later on may also hold column or a range.

Chunks are typically written to in a byte by byte sequence but can be written to in terms of a 'memcpy' like situation where all the lines are the same.  This makes writing data to an instruction relatively unintuitive since you typically have to do byte by byte.  This then complicates situations by introducing the whole endianness to the program.  The fix is somewhat simple.  You should write in the respect of no endianness and just the default of the system, underneath it'll convert it to something that is reasonable by storing the endianness as a parameter in the file that it'll use to convert in the case the system's don't match.

An easy way to write a sequence would be like;

```c
int data = 2;
// 0 being the line number
write_chunk_seq(chunk, sizeof(int), &data, 0);

// which is the same as
int data = 2;
WRITE_CHUNK_RAW(chunk, data, 0);
```

## Types

Bacon supports concrete types for values noting that it has no concept for variables (similar to how labels work in python - in some ways).

There are a few type 'classes (or groups);

- Primitives: int/uint (including sizes), boolean, and floating point numbers
- Core: big int/float, strings
- Core Collections: Array and Tuples
- Miscellaneous: User Data, `void` (explicitly no type/value -- therefore it is kinda like null), Tuple Map

### Arrays, Tuples, and Tuple Maps

Arrays have 2 modes; non-homogenous and homogeneous.

Homogeneous (all members have the same type) is more space efficient since it stores the type once, arrays also are very efficient at indexing.

NonoHomogeneous (all members have the same size - 9 bytes - but can be different type wise) this method is less space efficient but more flexible in terms of different types.  Often the size requirement doesn't impact primitives (since they all fit in a register) but will effect most other types.

Tuples store the type of each member with the member but don't force them to all be the same size, this allows more efficient storage.

Tuple maps are effectively a map ontop of a tuple / array, that is they allow you to encode offsets into the map.  The map is a binary tree (inplace) that is constant in size implemented typically using a key sorted array.  The `O(log(n))` access time is more efficient than a normal tuple access of `O(n)`.  This isn't entirely true because it does use a hashmap lookup as an initial guess (rather than the centre point) but it is worse case `O(log(n))` still and is more common to be that than the amortized cost of `O(1)` for your standard hashmap.

Tuple maps also allow you to run functions based on a key effectively allowing properties!  This is mostly optimised out to just offsets for things that are simple like; `this[0]` or `this.x`, these offsets are actually implemented as a chain of offsets allowing stuff like; `this.data[5].other` and technically you can even have a call in there since it'll encode it as a function with an offset i.e. `this.data[5].other()` just realise that it is impossible to optimise it past that i.e. `this.data[5].other().foo` can't be converted to offets and has to formed to a function another case is one with arguments i.e. `this.x(1, 2)` has to be done inside a function.  Tuple maps always return a reference to their argument and you as a language designer can use this to allow mutable properties.

### GC - How does it work

It is a standard mark and sweep aimed for lower latency applications rather than lower throughput (i.e. games where the GC wait time is worse than it occurring consistently more).

There are a few smaller things however;

- Arrays and Tuples are kept alive if they are referenced OR one of their children are referenced.  This allows us to not have to box tuples and be generally more memory efficient at the slight inefficiency of having to keep certain bits of memory alive.
  - There are plans to instead detect when this happens and move the memory to a new location and update all references to it, this would make the GC more expensive so would probably have to be an opt in feature.
