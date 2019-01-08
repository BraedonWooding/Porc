# Contributer Guide

> This is gonna be quite verbose before I come back and make it clearer, mostly just a mind dump for now sorry!

Hey!  This language was designed both to serve the purpose of being a better shell scripting language, show that C can still create modern programs and illustrate a few ideas I had for making a more modern C.

Hopefully this guide helps cover the few 'oddities'

## Compiler Pipeline

### Tokenization

You will notice that the two major token files are auto generated (`token_data.incl` and `token_list.incl`) these are generated automatically by `generate_token_data.py` using the `tokens` file.

Basically `token_list` just contains each token (comma separated) basically for the uses of creating an enum from them.

Whereas `token_data` contains conversions both from string to tokens and backwards (both the 'name' format and a more typical 'output format' i.e. name being `TOK_EQL` and 'output' being `=`) we use an in-memory trie for conversions (both for efficiency, ease of use and just simplicitly).

`token_data` also stores the ability to match against tokens very easiily (though sadly we have to basically do a memcmp(0) since we avoid pointers - I'm looking at ways to perhaps improve this probably using unique_ptrs or similar).  This is basically by representing a trie in static memory.

> Note: the want to improve it is not for efficiency reasons (a double ptr comparison is negligible) but more for readability purposes.

## AST

Token stream gets converted to an AST, this AST is a recursive like structure (which is unoptimal for optimisations) since expressions like; `1 + 1 * (2 + 4 / 2)` have a lot of folding.

> Terminology note: 'folding' is where you have something like this `ConditionalExpr(LogicalOrExpr(LogicalAndExpr(AdditiveExpr(...Constant(1)))))` where a very simple AST node is complicated due to a complex tree structure.

## AST Flattening



## AST Dev Modes

You can print out the AST in multiple formats; the three useful ones are (so far the only supported ones);

- JSON (perhaps useful since you could just parse porc files in background and use the produced AST to give some information)
- Simplified JSON (basically no extraneous `"` looks a bit like the python AST)
- Tree structure (prettier but not suitable for large ASTs)

How these all print out is effectively the same, there is a single virtual printing function that returns a special 'string' object containing meta data about the class.  For example if you had the code `println("Hello World the numbers 1 + 1 = ${1 + 1}, which is amazing!");` it may form the following object meta data (json rep);

```json
{
    // the name of the object
    "name": "FuncCall",
    "pos": [[0, 0], [0, 71]],
    "data": [{
        "func": "println",
        "pos": [[0, 0], [0, 7]],
        "args": [{
            "name": "StringInterpolation",
            "pos": [[0, 7], [0, 71]],
            "data": [{
                "name": "Constant",
                "data": [{
                    "type": "string",
                    "value": "Hello World the numbers 1 + 1 = "
                }]
            }, {
                "name": "AdditiveExpression",
                "pos": [[0, 44], [0, 49]],
                "data": {
                    "lhs": {
                        "name": "Constant",
                        "pos": [[0, 46], [0, 46]],
                        "data": [{
                            "type": "int",
                            "value": 1
                        }]
                    },
                    "op": "+",
                    "rhs": {
                        "name": "Constant",
                        "pos": [[0, 48], [0, 48]],
                        "data": [{
                            "type": "int",
                            "value": 1
                        }]
                    }
                }
            }, {
                "name": "Constant",
                "pos": [[0, 50], [0, 71]],
                "data": [{
                    "type": "string",
                    "value": ", which is amazing!"
                }]
            }]
        }]
    }]
}
```

Which as you can see is quite large, most of this useful in various different contexts but not all of it, so it is trimmed when converted to the formats!

Basically the conversions that occur are;

- For JSON: Strips away superfluous (extra) `[` `]` that can surround some maps.  As well as writes variables as `"var_name": data` rather than a much longer way (having to describe it as a type member and a lot more complexity).

An example of the following program is shown in each of the outputs below;

```rust
number_guesser = fn (to_guess: uint, bounds: std.range) uint => {
    guesses = 0;
    guess = to_guess + 1;
    while (guess != to_guess) {
        print("Enter guess (${bounds}): ");
        guess = uint(std.get_input());
        if (!bounds.is_within(guess)) println("Out of bounds");
        else {
            guesses++;
            if (guess == to_guess) break;
        }
    }
    return guesses;
}

main = fn (void) => {
    guesses = number_guesser(std.rand.int(0, 100), std.range(0, 100));
    println("You got it in ${guesses}!");
}
```




