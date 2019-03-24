# Porc's Style Guide

This is built so that all Porc code can look similar; this is aimed to accomplish the same thing as Go/C#'s style guide.  It also aims to not care too much about formatting and just recommend some formatting suggestions but more cares about naming and structure.

## Naming

- All locals must be lower snake case (i.e. `hello_world`)
- All globals must be upper camel case (i.e. `HelloWorld`)
- Namespaces/Modules must be lower case and a single word (i.e. `std` or `io`)
  - Preferably also just a few letters (i.e. `obs` instead of `obsidian`)
- Functions must be lower camel case (i.e. `helloWorld`)
- Macro Functions must be upper camel case (i.e. `HelloWorld`)
- Constants must be upper snake case (i.e. `HELLO_WORLD`)
- Typenames (structs) must be upper camel case (i.e. `HelloWorld`)
- Struct members/functions follow the usual rules for locals/functions (i.e. `hello_world` for members and `helloWorld` for functions)

## Functions

- All function arguments and the function return value should have types
- Functions should be declared like `fn myFunc() { }` in the global namespace and like `myFunc = () => { }` in structs and for lambdas

## Structs

- All struct members should be typed
- Try to avoid static members that aren't constant.

## Control Flow

```rust
// prefer no parentheses if followed by a block
if x {

}
while x {

}
for x in y {

}
// prefer parentheses if followed no block
if (x) y;
while (x) y;
for (x in y) z;
// prefer empty block to single semicolon
while (x) {} // instead of while (x);
for (x in y) {}
```

## Function folding

```rust
// function folding can make code look nicer
using cmd.*;
results = for line in wget("...") -> grep("Name: ") {
            str.toUpper(line) -> str.trim("Name: ")
          };
// compared to
results = for line in grep(wget("..."), "Name: ") {
            trim(upper(line), "Name: ")
          }

// but it can also make code look more obscure and ugly
run_function = func() -> x() <- y() -> z();
```

- All arrows should point the same way and there should never be a 360 turn.
  - i.e. `x -> z -> a` and sometimes `x -> z -> b <- a <- q` but not `x -> z <- a -> b`
- Always start off with writing it folded and only use arrows if the statement needs it
- If you are using more than 4 trailing statements than maybe there is a better way

## Suggested Formatting

Feel free to modify these as you wish but keep it consistent.

- Uses spaces for indents and 2 spaces at that
  - Porc code is typically quite short and simple and 2 spaces helps keep it concise
- No spaces after function names i.e. `helloWorld()` not `helloWorld ()`
- Spaces after control flow i.e. `if (x)` not `if(x)`
- Keep lines to 80 characters maximum
- Keep functions short
