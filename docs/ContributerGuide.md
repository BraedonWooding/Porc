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

We flatten all math operations to a special type of AST node optimised specifically for math, this allows much easier optimisation and code generation.

`(1 + x + 1) * (2 * 4 - y + 1 / z / y())` =>

For example the AST will look something like this before flattening (excluding folding);

> Note: representing constants/identifiers as `x`, `1` and so on...

```rust
MultiplicationExpr(
    Parentheses(AdditionExpr(
        AdditionExpr(1, x),
        1
    )),
    Parentheses(AdditionExpr(
        SubtractionExpr(
            MultiplicationExpr(2, 4),
            y
        ),
        DivisionExpr(DivisionExpr(1, z), FuncCall(y))
    ))
)
```

We can then make this even better to optimise like (flattening it);

```rust
MultiplicationExpr(
    // see here: how we can have multiple in a group
    // the extraneous group is removed here
    AdditionExprGroup(1, x, 1),
    MathGroup(
        Add(
            Sub(
                Times(2, 4),
            y
        )
        )
        Times(2, 4)
        Sub(y)
        Add(Div(1, z, FuncCall(y)))
    )
)
```

Now this is much clearer to become;

```rust
MathExpr(
    Times(
        Add(2, x),
        Group(
            Sub(8, 4)
        )
    )
)
```
