# Porc's Type System

Porc has a type system that most likely you haven't interfaced with previously.  Before we discuss it first I want to note that Porc was built and designed around having a properly dynamic language without the type problems it typically brings.

## Q: Is it dynamic or static

Well I think the problem lays in the fact that both those words aren't really properly defined and some things blur the lines between them typically though dynamic resolves type references at runtime where as static resolves type references at compile time, typically 'runtime' refers to the last point required to resolve it (i.e. wait as long as we can).

Porc carries a bit of both as you'll see, how about you judge for yourself :) (I'll add my opinion of which one it is at the end).

## Problem it aimed to solve

I want to give you a good reasoning of why I approached the problem this way (really should help you understand the type system).

I aimed to solve the following;

### Side-effects

For example;

```C
add = (a, b) => {
    c = a + b;
    stdout = std.stdout();
    stdout.write_ln("\{a\} + \{b\} = \{c\}");
    return c;
};
```

Now what if we fail to open `stdout`?  Also the fact that this prints out to stdout is a side-effect of this function ('subjective' yes) so we want to represent these side effects in the type system.  That way we can control them if need be, for example change stdout to stderr or a file or maybe just an array.  This is a boon to testing mainly and promotes better code design (want to change that file out to a database, just requires a short line or two rather than a redesign)

### Incorrect Types

Passing in wrong types should be handled properly!  We shouldn't need to have checkers with comments and so on (like python).

This helps massively with big projects as it not only prevents the classic python bugs where you pass an incorrect type for example `get_centre_from_id(centre)` rather than `get_centre_from_id(centre.id)` (actually happened to me in an assignment :( our tests didn't pickup on this since our tests were just backend).

### Slow

Typically dynamic languages are slow (python mainly in this and LUA - non jit to a degree) but why is this??

Well it is mainly that when I do `c = a + b` I need to really transpose it down to `c = a.__add__(b)` (happens with lua meta-tables, though there are also addition commands that are more optimised) typically people say stuff like python can convert this to fast arithmetic operations and that's true but its still overhead and in this example `c = a.doX(b, d)` it has to somehow look up this `doX` method on `a` rather than having it already mapped.  All these little things I think are to blame for the speed!

So Porc aims to provide the compiler/byte code VM with as much information as it can this causes shortcuts to be faster and typically eliminates a lot of these calls.

For example `c = a + b` (where `a` and `b` are integers) creates the following bytecode rep

```assembly
new_var $c %int
arith_add %int 2 $a $b $c
```

Which literally just decomposes to something like (in C);

```C
int c;
c = a + b;
```

We follow LUA in the fact that our interpreter is extremely simple (and built in C!).

## The solution

Hopefully all this background will help reduce questions you may have about the implementation.  Keep in mind a lot of this syntax is EXTREMELY experimental and as usual syntax is really irrelevant when talking about type systems.

First we have a bit of terminology; `type variants` is just a fancy way of saying that a type can be either one of the types (i.e. `int | float` means int or float), `type invariant` means that rather than explicitly restricting the type you are restricting the 'view' of the type that is `int ^ float` means that the type can either be int and/or float.  Now this and/or is the tricky part of the type system but actually is extremely powerful.

For example let's say we want to build a generic 'add' function;

```rust
add = (a, b) => a + b;
```

Now what are the types of this?  Well if you leave out types Porc automatically 'tries' to fit the types as much as it can and in this case will come up with this maybe?

```rust
add = (a: any ^ op.add, b: any ^ op.add) any => a + b;
```

Now in actual fact this isn't exactly what it would produce, since this would allow `add(1, "hello")` and so we need to make sure that add binds to both (while also allowing `a` and `b` to be different types to allow `add(1, 2.5)` and `add("hello", 'h')`).  Now if we could just make `b` `b: any ^ op.add ^ a` but that would require `b` to be viewed as the same type as `a` which would break the second case, in this case we just want an `op.add` that supports `a` and `b`.

```rust
add = (a: any ^ op.add, b: any ^ op.add(a, b)) op.add(a, b) => a + b;
```

Yes that is a little bit of a recursive function but it's allowed since the compiler can clearly see you are just applying a function check.

Basically if you have something like `a: type ^ function` then it know's what you are trying to do and just makes sure that the type implements that function (that is the type is 'viewable' as that function), if you call the function then you aren't actually calling the function but rather making sure that the function *is* callable with those types, this in fact actually returns a type which in this case we can use as the return value!  Now as a human this is a bit verbose so you can simplify it like;

```rust
add = (a: any ^ op.add(a, b), b: |a) ~a => a + b;
```

Now `~a` means unary complement in this case we are saying the type is `any` that implements `op.add(a, b)` so the complement is `op.add(a, b)` that implements `any` (`op.add(a, b) ^ any`) which is equivalent to `op.add(a, b)`.

> And yes that does mean that the order of `^` matters you should read it as 'implements' so `int ^ float` means `int` that implements `float` which means that you can take in integers but you can also manipulate them as floats (more useful when talking about it in terms of `type ^ str`).

What does `|a` mean?  Well sometimes we want to 'refresh' the type of a constraint in this case we want to let `b` have a different type (but be under the same constraints as `a`) how do we do this?  Well we want to create a parallel type, and well `|` means parallel.

In reality I probably wouldn't have used `~a` here since the way it works is a little 'magic' in this instance (there are more reasonable uses) but I felt it was wortht o show that you don't need to always add a lot of verbosity to add types.

Okay so this solves the 'incorrect' type problem for example in my original problem I had something like;

```python
def centre_by_id(self, centre_id):
    for centre in self._centres:
        if centre.id == centre_id: return centre
    return None
```

> Yes I know this isn't very pythonic but I want it to be readable by those who don't know python so I'm going to avoid using a list comprehension and `or` or some kind of function.

In Porc it would look like;

```rust
centre_by_id = (self, centre_id) => {
    for centre in self._centres {
        if centre.id == centre_id  break centre;
    } else null // if for loop doesn't break
    // last statement without semi-colon returns
};
```

And adding types (going to leave out self since that's irrelevant in this case) and presuming `centre.id` is an int...

```rust
centre_by_id = (self, centre_id: any ^ int) Null | Centre => { /* ... */ };
```

Note: how we also fix the 'null' problem by making it part of the type system, null is the only type that you can't '^' (well currently) since you can't 'view' a type as nothing that makes no sense.  Also note that while this fixes the case where you pass an incompatible type it doesn't fix the case where you do `centre_by_id(3.02)` since that'll just find the one with id == 3.  However this case is a lot rarer and may even pass static type checks (atleast errors, will exist in warnings), we can force more strict rules by compiling Porc with `--strict-types` which will just do `centre_id: int` as that is the 'lowest' point it can resolve, it won't effect the above add function since it can't resolve lower.

Basically Porc by default tries to make the types as wide as possible to allow you to be as dynamic as you wish, however dependent upon your style you may want to narrow that using `--strict-types`.

### Recap

So we fix types by basically having 2 kinds of types that the compiler does mostly automatically (you of course can do itself if you want) `invariants`, `variants`.  Technically `a: int` is a variant just with no other 'variants'.

But what about side effects...?

### Side-Effects

I haven't really covered side-effects because they are actually just covered by the type system really simply.  I'll just begin with an example;

```rust
add = (a, b) => {
    c = a + b;
    println("{a} + {b} = {c}");
    return c;
};
```

In this case we have used a function `println`, how is the function defined well it looks something like this;

```rust
println = (msg: any ^ str) => {
    out = @context(GLOBAL, main_out);
    out.write_ln(msg);
};
```

> In reality there is a bit of code to not re-open stdout if it is already open in the current block but I left that out for readability.

The string interpolation is handled at a higher level so we just have to write out the line.  `@context` is the 'magic' here that handles our context quite well!  Basically we have characterised that there are two types of effects;

- 'unconventional' side effects (like it writes the change to the database or updates some other person's 'ability')
- 'conventional' side effects it just does some kind of 'IO' work that we may want to re-route.

In reality the second one is the only one we care about!  The other one can be handled by context but typically is just handled but having comments or just having better defined functions.

Context is quite simple; it is kinda similar to the `global meta table` for LUA, but you can actually make different context tables.  For example a bootloader for Porc could look like;

```rust
GLOBAL = @create_context();
@export_full(GLOBAL); // make it available to all programs
@set_context(main_out, std.io.stdout());
@set_context(main_in, std.io.stdin());
@set_context(err_out, std.io.stderr());
@resolve_late(LAST, @resolve_context_links(GLOBAL))
res = @resolve_late(LAST) <- main;
@drop_context(GLOBAL);
return res; // or set ret_val properly
```

Context is actually just a dictionary, `@resolve_late` just lets you set the resolution time for a component, in this case we want to link up main/`@resolve_context_links` as late as we can (since well the compilation of the bootloader will occur possibly much before the compilation of your program) also note that what resolving links does is sets `out` and so on to the location of the variable `main_out`, this means that we DON'T need a hash or something like that since we have the location of `main_out` (not what it holds) meaning we'll update along with all context changes.  Also note that contexts AREN'T garbage collected (garbage collection is actually quite a complex topic that is covered under [memory](memory.md))
