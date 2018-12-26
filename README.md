# Porc

> A powerful extensible language built for rapid development.

> Powerful typing system that simultaneously protects against invalid types while requiring no types!

> An extremely powerful constraint like system to go hand in hand with the type system.

> Proper macro support that is built into the parser rather than being external!

> *WARNING* This language is extremely WIP don't expect anything to be working for a decent time (as a rough estimate from previous projects around 6k lines I would expect basic functionality it currently is sitting at around 3k) this is not including the time estimates for 

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

Are you interested?  Maybe check out the [Language Guide](LanguageGuide.md).
