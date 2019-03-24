# PIR (or PYRE) - Porc's Intermediate Representation

PIR is Porc's IR and is an odd mix between an AST, Bytecode (from something like Java/LLVM/C#), and 'Assembly'.

## The aim of this hybrid

- To be easily optimised
  - So we can perform some optimisations and still have a fast compile time but at the same time also allow more heavy optimisations
- Reduce the amount of full tree traversals that are needed to build the output
- Have a representation that is extremely similar to the final representation
  - Again this reduces the work, it also means that we could just produce the final representation in the case where we don't care about optimisation
- Require no type work to produce
  - A big cost of having the types be the way they currently are is that we would have to perform a full traversal once the tree is done to figure out what types aren't defined, this way we can track them as we build the IR.
  - Also it means that 

## Structure

There are two main important properties to the structure;

### `SECTION`

This groups functions and structures as well as common types

### `SUB_SECTION`

This is used for things like holding code, defining arguments, and etc...

## Unique Problems/Consequences

- We don't have mutable globals (out of `context`) so all functions outside of ones inside classes are automatically `pure`.
- We don't necessarily know the types of things and have to figure out the possible constraints
- Optimisations on AST trees are expensive (but efficient) where as optimisations on instruction sets typically aren't as effective but can be much cheaper

## 'Syntax' of PIR

Note: PIR doesn't have a syntatical representation since it is purely a binary one however we do often print out a psuedo representation (that is it can't be necessarily parsed but it can be printed) to aid with debugging.

## Fib Example

```rust
/* test.porc */

fib = (n) => n <= 1 ? 1 : fib(n - 1) + fib(n - 2);

fn main() {
  println(fib(2) * fib(9) + fib(14));
}

/* test.pir */

// This represents a `class` of some sorts (in this case a function)
FUNCTION fib /* Flags are combined with `|` */ GLOBAL | RECURSIVE | PURE | ASYNC_PURE
  FILE("test.porc")
  LINE_RANGE(3, 3, 1, 50)
  ARGS
    DEF n__0 /* no type specified */ LINE_RANGE(3, 3, 8, 8)
    DEF __ret /* return object */ LINE_RANGE(3, 3, 14, 50);
  END

  CODE
    LOAD __0 n__0
    LINE_RANGE(3, 3, 14, 19)
    TEST __0 <= 1 __L0
    LOAD __1 n__0
    LINE_RANGE(3, 3, 31, 35)
    SUB __1 1
    LINE_RANGE(3, 3, 27, 36)
    CALL fib __1
    PUSH __ret
    LOAD __2 n__0
    LINE_RANGE(3, 3, 44, 48)
    SUB __2 2
    LINE_RANGE(3, 3, 40, 49)
    CALL fib __2
    POP __3
    ADD __ret __3
    RET __ret
    LABEL __L0
    LINE_RANGE(3, 3, 23, 23)
    LOAD __ret 1
    RET __ret
  END
END /* End of class definition */

FUNCTION main GLOBAL | PURE | ASYNC_PURE | MAIN
  FILE("test.porc")
  LINE_RANGE(5, 7, 1, 11);
  ARGS
  END

  CODE
    LOAD __0 2
    CALL fib __0
    PUSH __ret
    LOAD __0 9
    CALL fib __0
    POP __1
    MUL __1 __ret
    PUSH __1
    LOAD __2 14
    CALL fib __2
    POP __3
    ADD __3 __ret
    CALL println __3
  END
END
```

## Assignment Example

```rust
/* assign.porc */

fn main() {
  x = 2;
  y: flt = 4;
  x += y;
  x = 9;
  y = y + x; // same thing as y += x
  println(x);
  println(y);
}

/* assign.pir */

// This represents a `class` of some sorts (in this case a function)
FUNCTION main /* Flags are combined with `|` */ GLOBAL | PURE | ASYNC_PURE | MAIN
  FILE("assign.porc")
  // Not going to include line ranges as it is too much work to figure them out
  ARGS
  END

  CODE
    // we use a new variable for each assignment (static single assignment)
    // this makes it easier to see unused variables
    DEF x__0 int
    LOAD x 2
    DEF y__0 flt
    LOAD y 4
    DEF x__1 int
    LOAD __0 x__0
    ADD __0 y__0
    LOAD x__1 __0
    DEF x__2 int
    LOAD x__2 9
    DEF y__1 flt
    LOAD __1 y__0
    ADD __1 x__2
    LOAD y__1 __1
    CALL println x__2
    CALL println y__1
  END
END /* End of class definition */
```

This can then get optimised into

```rust
// ...
  CODE
    // note: using temporaries and using registers
    LOAD __R0 2
    LOAD __R1 4.0
    ADD __R0 __R1
    LOAD __R0 9
    ADD __R1 __R0
    CALL println __R0
    CALL println __R1
  END
// ...
```

Then of course you can have constant folding to optimise away the constants but that is a significantly less interesting transformation.

## Loops

``` rust
/* loops.porc */

fn main() {
  for (i in 0..10) println(i);
  for (i in [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]) println(i);
  for (i = 0; i < 10; i++) println(i);
  i = 0;
  while (i < 10) {
    println(i);
    i++;
  }
}

/* loops.pir */

FUNCTION main GLOBAL | PURE | ASYNC_PURE | MAIN
  ARGS
  END

  CODE
    LOAD __0 0
    LOAD __1 10
    LOAD __2 1 /* step */
    LOAD __3 false /* inclusive */
    CALL range.new __0 __1 __2 __3
    LOAD __4 __ret
    DEF i__0 int
    FOR_IN i__0 __4 __L0
      CALL println i__0
    LABEL __L0 // stop for in

    LOAD_STATIC_ARRAY __5 10
    LOAD_BATCH __5 /* size */ 10 /* args */ 0 1 2 3 4 5 6 7 8 9 10
    DEF i__1 int
    FOR_IN i__1 __5 __L1
      CALL println i__1
    LABEL __L1 // stop for in

    DEF i__2 int
    LOAD i__2 0
    WHILE_COND i < 10 __L2
      CALL println i__2
      ADD i__2 1
    LABEL __L2

    // the other while loop is identical to above
  END
END
```

## Opcodes

Opcode instruction setup

```C
|---------------------------------------------------------------|
| OpCode (6 bits) | OpFlags (2 bits) | Data (0, 32, or 64 bits) |
|---------------------------------------------------------------|
```

OpFlags are completely dependent on the opcodes but typically the first one is used for `Q` to determine if we need to query types.

### Syntax

This syntax just illustrates opcodes in a concise manner;

`NAME [OPT_FLAGS] ARGS()`

- `NAME` is just the opcode name
- `[OPT_FLAGS]` are optional flags i.e. `[Q F]`
- `ARGS()` are just the arguments i.e. `V()` (variable) `R()` (register) `C()` constant `Q()` to be defined by the presence/absence of flags, `L()` is a label

This can be constructed like `LOAD_C|Q __R0 42`

### Move Data Opcodes

- `MOV [R] V() Q()` Move data from some location to a variable
  - `R` designates that the second argument is a register, else it is a variable
- `LOAD [Q] R() V()` Move data from a variable to a register
  - `Q` works like you would think it would (the register has an undefined type)
    - typically register's carry the type of their input data but this signifies the input data has no type.
- `LOAD_C [Q] R() C()` same as above but for a constant
- `LOAD_R [Q] R() R()` same as above but for a register (typically useful to move arguments).
- `SWAP [R] Q() Q()` swaps two locations of memory
  - `R` means swaps two registers else it means swap two variables.
- `PUSH R()` push R onto the stack to save it
- `POP R()` pop the top register of the stack to unsave it and move it into R()

### Arithmetic Opcodes

- `SUB [O] R() R()` subtracts second from first (i.e. first - second) then places result into first
  - `O` places result into second
- `ADD [O] R() R()` does the same thing but for addition
- `MUL [O] R() R()` does the same thing but for multiplication
- `POW [O] R() R()` does the same thing but for exponentiation
- `DIV [O] R() R()` does the same thing but for division
- `MOD [O] R() R()` does the same thing but for modulus

> All these opcodes also have an `I` and a `F` variant i.e. `SUB_I` and `SUB_F` to denote that it is just integers or just floats.

### Comparison Opcodes

- `GT [C O] R() Q()` checks if arg1 > arg2 and sets arg1 to the result.
  - `C` means that `Q()` is `C()` else it is `R()`
  - `O` writes to the second rather than the first
    - Useful in the cases where you want it to write to the second argument rather than the first
- `LT [C O] R() Q()` same as above but with arg1 < arg2
- `EQ [C O] R() Q()` same but for arg1 == arg2
- `NEQ [C O] R() Q()` same but for arg1 != arg2

### Jumping Opcodes

- `JMP [F] L() R()` jumps to label in the case that `_R0` is truthy.
  - `F` just jumps in the case that it is not truthy.
- `GOTO L()` jumps to label
- `LABEL L()` defines the position of label

### Special Opcodes

- `RET [F] Q()` returns with a designated object
  - `F` means the return object is just the size (or smaller) than a register
    - i.e. fast return
  - else `Q()` is `V()` and refers to a long lived object.

## Important things about instructions

- Things like `DEF` aren't an opcode and exist in the function headers and global/context headers
- Arithmetic/Comparison can only occur on registers
- There is 0 assurance that a call won't modify registers so storing registers before calls is needed
  - The only case where this isn't true is technically when calling a function it won't edit more `_A` register's then it needs however this is a really expensive calculation to check (that is checking all child function calls and all their calls and getting the highest _A set) and is technically a halting problem (however that isn't the concern more than just a really large traversal)
- `PUSH`/`POP` should be used to save variables.

### Side note on pushing/popping

Interestingly enough instead of pushing/popping `n` in the above example we could actually just have it like this;

```rust
// save `n` this way by storing it in the variable meant for the return value
MOV_R|Q 0 __A0

// the fib call
CALL 0 1

LOAD|Q __A0 0
MOV_R|Q 0 __Rret
// and the rest
```

Pushing/Popping is relatively cheap but could be more expensive then a variable (which would just require setting a value).  It may actually be cheaper to push everything (including the return value) such that now the cost for the function is actually 0, however you pay the push storage costs so in reality it is the same (but arguably it is cheaper once you get to the point where the function stack frame can't fit on the 'stack' and has to be heap allocated).

## Data

Data is stored like this;

```C
|------|------|
| Type | Data |
|------|------|
```

Yeh it really is that simple!  Well in theory...

The complexity is due to the fact that `type` isn't necessarily 1 or 2 bytes that depends on the value of the right most byte (and in actual fact we store the number the wrong way around) also data's size is undefined and can only really be queried from the type information.

If the right most byte (which is actually the left most as you have to rotate) is < 127 (that is it is form `0xxxxxxx` where `x` can be either `0` or `1`) then the type is only 1 byte and can be a primative (only a small subsection of it is a primative, the rest are just typically common std classes).  This is used because it means storing ints are quite cheap.

Else the type size is only 2 bytes, furthermore each `struct` gets its own type and the value of the type is actually the offset into an array of extra type information.  What this means is that storing type information is typically much cheaper then other languages (where it represents a pointer of around 8 bytes on modern systems) but querying can be reasonably expensive.

For this reason we mostly avoid any type queries and often resort to just explicitly resolving offsets for things like member values and functions, the only real point we have to resolve is for example interfaces where we have to query for the function we are looking for which often is an O(N) operation in the worst case (in reality we put any potential interface methods at the start of a struct type).

However various optimisation methods exist that override this cost such as...

> Note: all the below code around optimisation is generated by the compiler in a form that is somewhat equivalent as if you wrote it.

### Context Function Resolution

For example if we have some code like this;

```rust
CanFly = (void) :: abstract {
    can_fly: bool;
    fly: fn (self) | None = None;
};

Bird = (name: str, age: int) :: {
  can_fly = true;
  fn fly(self) {
    println("This bird ${name} can fly despite being ${age} years old!"):
  };
}

Foo = (animal: CanFly) => if (animal.can_fly) animal.fly() else println("This animal can't fly");

Foo(Bird("Jeff", 3));
```

Then it is silly to have to do function look ups so what we do is we make a substruct that just holds the functions and pass that as well which turns the function `Foo` into;

```rust
// we use `__` for unique naming
CanFly__TupleSet = (can_fly: bool, fly: fn(self) | None);

Foo = (animal: CanFly, __set: CanFly__TupleSet) => __set.can_fly ? __set.fly(animal) : println("This animal can't fly");

// extra parenthesis for tuple
Foo(Bird("Jeff", 3), (Bird.can_fly, Bird.fly));
```

This can be a huge efficiency boost (sometimes even `animal` can be removed!) this means that you don't pay for the lookup costs however you are paying for a tuple construction which isn't great...

### Precomputed Offsets

This is another way it can be solved (in some cases) this is typically preferred for release applications that are closed (that is all code is compiled and no new code will be injected) since it avoids the tuple construction, what it does is precompute the offsets for the two functions and place it into the context in code it looks something like;

```rust
Foo = (animal: CanFly) => {
  can_fly, fly = @context(Foo, (can_fly: bool, fly: fn(self) | None));
  return can_fly ? fly(animal) : println("This animal can't fly");
}

// note: if we didn't have this then Foo call would raise a compile exception
@context_set(Foo, (can_fly=Bird.can_fly, fly=Bird.fly));
Foo(Bird("Jeff", 3));
```

So in terms of code generated what is the difference?

#### Pros/Cons of Precomputed Offsets

- It is more efficient it doesn't involve a tuple construction
- It is simpler
- If your function is recursive you have to maintain the same context
  - Meaning for some recursive functions it is not applicable
- Doesn't work particularly well for async functions
  - Unless you have async contexts (basically a separate context for each async call)
- We still have to get the offsets from the context tuple (which is basically equivalent to getting the offsets from the passed tuple) so we only save on construction
- For recursive functions that maintain the same context it is significantly cheaper
  - Then again most likely it'll just pass the same tuple over and over again when it realises they are identical (maybe not on lower optimisation levels)
- Compile time maybe be a tad longer for the second method (but really compile times should be fast regardless)
