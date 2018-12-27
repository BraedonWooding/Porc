# Porc

> A powerful extensible language built for rapid development.

> Powerful typing system that simultaneously protects against invalid types while requiring no types!

> An extremely powerful constraint like system to go hand in hand with the type system.

> Proper macro support that is built into the parser rather than being external!

> *WARNING* This language is extremely WIP don't expect anything to be working for a decent time (as a rough estimate from previous compiler projects around 6k lines I would expect basic functionality it currently is sitting at around 3k) this is not including the time estimates for the byte code interpreter.

Example of the scripting power;

```C
// our package manager at work
@import_package("BraedonWooding/cmd-porc", *);
echo(stdout()) <- grep("A") <- wget("http://some_address.com/some_file.txt") -> sort(.Reverse);
```

Example of the typing system;

```rust
add = (a, b) => a + b;
// above is identical to
add = (a: any ^ ops.add(a, b), b: |a) ~a => a + b;
// which is just
add = (a: any ^ ops.add(a, b), b: any ^ ops.add(a, b)) ops.add(a, b) => a + b;

println(add(3, 5)); // 8
println(add(3.0, 2)); // 5.0
println(add(2, 3.0)); // 5.0
println(add("Bob", 2)); // Compile Time Error: Invalid Types
println(add("Bob", "Jim")); // "BobJim"
```

Are you interested?  Maybe check out the [Language Guide](docs/LanguageGuide.md).

## Why C and not C++

C/C++ are really the only languages I could see myself using for a compiler that is performant reliant (not haskell/ocaml mainly due to me not having any sort of knowledge of these languages).

1) Compiler's don't really benefit from the standard library that C++ has
  a) Things like vectors/hashmaps often have to be hand rolled anyways to fit the needs of the language (for example LUA's weird tables or python's weird dictionaries)

2) Compiler's have a lot of boiler plate code that grossly outweighs the extra boilerplate that C has compared to other languages
  a) for example look at AST.c and tell me where C++ would grant any sort of meaningful decrease in code size - I'm waiting
  b) This is kinda the biggest point for me, a lot of the code would look pratically identical (except for the whole baseAST stuff) regardless if it is C/C++
  c) I can use generators such as `./generate_token_data.py` if I feel the boiler plate code is similar enough and warrants it.

3) C is much more portable than C++
  a) though since I use extensions so much this point is mute

4) I'm more familiar with C
  a) I have probably around 1-2 yrs of primarily C (with it being maybe my 5-6th language I've picked up) compared to about 6 months of primariliy C++

5) C is much much much easier to install and use as a tool chain

6) C is much nicer to use as a FFI, this means that we effictively inherent FFIs for the following languages without any effort (much effort I should say); Python, LUA, C++, Perl, and so on...
  a) Really the only languages that aren't so nice is Java (which is almost always a pain) and C# (which granted isn't much easier either but we can use managed C++ and a C binary which while is quite a bit of work is only 1 language to 4+).
