# Porc

> A powerful extensible language built for rapid development.

> Powerful typing system that simultaneously protects against invalid types while requiring no types!

> An extremely powerful constraint like system to go hand in hand with the type system.

> Proper macro support that is built into the parser rather than being external!

> *WARNING* This language is extremely WIP don't expect anything to be working for a decent time.

Example of the scripting power;

```python
# definining imports
cmd :: @import(.GIT, "BraedonWooding/cmd-porc", "release");

# grab from the global context table stdout
stdout :: global.stdout;

echo(stdout) <| (grep("A") <| wget("google.com") |> sort(.Reverse));
# which is identical to;
echo(stdout, grep("A", sort(wget("google.com"), .Reverse)));
```

Example of the typing system;

```python
add := (a, b) => {
    return a + b;
}

# is identical to
add := (a: $A, b: $B)->$R => { return a + b; }

# which can be written as
add := (a: $A, b: $B)->$R => { = a + b; }

# and of course compressed
add := (a, b) => a + b;

println(add(3, 5)); # 8
println(add(3.0, 2)); # 5.0
println(add(2, 3.0)); # 5.0
println(add("Bob", 2)); # Compile Time Error: Invalid Types
println(add("Bob", "Jim")); # "BobJim"
```

Are you interested?  Maybe check out the [Language Guide](docs/LanguageGuide.md).

## Quick Contributer Guide

- This project is a mess right now as I just work towards a v0.1.
- We use C++17
  - I'm in the flows of updating right now
- Styleguide is Google's find it [here](https://google.github.io/styleguide/cppguide.html)
  - Mainly just because it's very simple and straightforward
  - Only changes are;
    - Use C++17 instead of C++11
    - Templates are fine if used for functions avoid templated classes.
      - I'll write up a post about this some other time but for now just take it as it is.
- Compiling on High Sierra; `cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=$(which clang++) -DCMAKE_C_COMPILER=$(which clang) ..`
- Compiling on Windows; `cmake -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 16 2019" -A x64 ..`
  - NOTE: You can change the version inside `-G` and the architecture of `-A` if you want it should work with any reasonably modern compiler
- You need the following libraries (just added to a folder called lib will work);
  - [catch](https://github.com/catchorg/Catch2)
  - [CLI11](https://github.com/CLIUtils/CLI11)
  - [expected](https://github.com/TartanLlama/expected)
  - [json](https://github.com/nlohmann/json)
  - [rang](https://github.com/agauniyal/rang)
  - [fifo_map](https://github.com/nlohmann/fifo_map)

## Roadmap

- [x] AST
  - Done! :D
- [x] Parser
  - Done! :D
- [ ] Bytecode Instruction Set
  - Basics are setup
- [ ] Basic Bytecode Codegen
  - No type checking/types (that is almost purely optional and just trusts you kinda like in python)
- [ ] Interpreter
- [ ] standard library implementation
- [ ] Possibly JIT
