# Language Guide

Welcome to Porc!  Let's begin with the standard rite of passage; `hello world!`

```c
println("Hello World!");
```

This doesn't really show anything about Porc so a nicer example would be perhaps be a vector example (that is mathematical vector), let's begin with a non type vector instance;

```python
# a nice way to 'typedef' things
type vec is (x, y, z);

vec_add :: (a: vec, b: vec) => {
    return vec(a.x + b.x, a.y + b.y, a.z + b.z);
}

# use 'with' to register a context variable
# str.buf is a mutable string that can be written to
# sidenote: str.ref is a reference passed string that can be changed
#           in this case its better to do it this way since it more
#           matches a file structure and is cheaper :).
vec_print :: (a: vec) => with (out: io.ostream | str.buf = global.stdout,
                              endl: char = io.line_sep) {
    println("${a.x}, ${a.y}, ${a.z}") with out=out, endl=endl;
}

main :: () => {
    v1 := vec(1, 2, 3);
    v2 := vec(0, 1, 4);
    v3 := vec_add(v1, v2);
    unit.assert(v1 == (1, 2, 3));
    unit.assert(v2 == (0, 1, 4));
    unit.assert(v3 == (1, 3, 7));
    # a dynamic string buffer (grows)
    out := str.buf.dyn();
    # use `with` to set context variables
    # you don't have to do the `out=out` if the variable name matches one
    vec_print(v1) with out;
    unit.assert(out == "1, 2, 3");
}
```

## Style Notes on Context Variables

Context variables should be used for anything that is a side effect of the method, typically stuff that is more suspectible to changes in implementation details.  For example the vec print method only requires a single vector the fact you can change the location and what character to print at the end of the line aren't related to printing the character but rather the side-effect so its 'better' to put them as context variables.

Or in other terms;

> Context variables for anything that has a strong default that typically won't be changed

## More complicated types

Now one thing about our vector type is that it is C styled which isn't particularly nice, rather we would want some way to 'bind' methods to a type rather than require this semi-ugly alternative.  This is however a really easy thing to do in Porc all you do is;

```python
# note the block!
type vec is (x, y, z) {
    add :: (this, other: vec) => {
        return vec(this.x + other.x, this.y + other.y, this.z + other.z);
    }

    # we can define a constructor using the type name
    str :: (this) => {
        return "${this.x}, ${this.y}, ${this.z}";
    }
}

# you can add more methods to already existent types like this
type str {
    # add a constructor for our vector from our str type
    vec :: (this) => {
        # add a comp time check! (or runtime if not able to compile time check)
        # filter(bool) will just cast the expression to bool which is a nice way
        # to check if the values are valid
        # to make it slightly more efficient we are going to just get split
        # to return a tuple rather than an array it should be faster since we
        # can then just call the vec method with our tuple and get it to check
        # types and that's it.
        split :: (this.split(',') with ret_tuple=true)
                 |> map(int.parse)
                 |> filter(bool);
        if split.len < 3 {
            return void;
        } else {
            return vec(split);
        }
    }
}

main :: () => {
    v1 := vec(1, 2, 3);
    v2 := vec(0, 1, 4);
    v3 := v1 + v2;
    unit.assert(v1 == (1, 2, 3));
    unit.assert(v2 == (0, 1, 4));
    unit.assert(v3 == (1, 3, 7));
    out := str.buf.dyn();
    println(str(v1)) with out;
    unit.assert(out == "1, 2, 3");
}
```

There is a shortcut to definining a method in another type and that is;

```python
type vec (x, y, z) {
    # typically to make it more explicit in these cases we like denoting the
    # `this` type.
    str.vec :: (this: str) => {
        # ... rest of code
    }
}

# which is identical to
type str {
    vec :: (this) => {
        # ... rest of code
    }
}
```

## Maybe Tuples

Porc doesn't like the requirement of having `(a,)` to denote a single item tuple.  So instead Porc has a compiler only type called `MaybeTuple` which is exactly what it sounds like it is a type that is maybe a tuple or maybe just the value type this only exists for single items it is denoted by `(a)`.

A way you can view it is like Quantum State (to a degree) the particle (or in this case tuple) has multiple choices of states (either tuple or just a single item) and as soon as you apply an operation to this maybe tuple you will collapse the wave function and make it choose which one.

This is done extremely intuitively and you will probably be surprised it is even happening since its very intuitive.  The way is very simple and that is as soon as you apply an operator to the tuple it will unwrap it and choose a value type for example `(a) + b` is going to become just `a + b` this is commonly used for arithmetic such as `(1 * 2 + 9) / 2`, the other way is when you either cast it, or apply it to a function.

For example if you have a function that is; `add :: (a, b) => a + b;` and you call it like; `add(1, (2));` it would choose the value type, in fact it ALWAYS passes it as a value.  This is because to cast from a value to a tuple is 'free' (extremely cheap bitflag swap and it often doesn't have to be done) so we almost always take the type as a value type the only case we don't is when the function specifies a tuple i.e.

```python
# note: in types (int) is always a tuple
#       I personally prefer Tuple[int] as its more clear but to each his own
swap :: (a: (int), b: (int)) => {
    tmp := a[0]; # use '*' to unwrap tuple
    a[0] = b[0];
    b[0] = a[0];
}

a := (2);
# going to upcast the 4 to a tuple then swap it
# also going to force the a to be a tuple
swap(a, 4);
# note: as said before casts are very cheap so we allow this kind of operator
unit.assert(a == 4);

# slightly more convoluted case
c := (9);
# this forces a to be an int
d := c + 2;
# this line however will error out!!
swap(c, a);
# this is because you have broken the rule of maybe tuples that is you can't
# make it both, it has to be one or the other.  What you most likely meant is to replace
# `d := c + 2;` with `d := c[0] + 2;`
```

A final thing to note is that you may argue that we can actually just cause the `c` to become a tuple because that is what we did for the 4!  The reason we don't is two fold;

- You can't change a variable's type from value to tuple you can only change a literal
  - And vice versa (if you had swapped teh `d := c + 2` and `swap(c, a)` line then you would get a similar error for converting a variable types from tuple to value).
- This could be a cause for bugs as people incorrectly do implicit tuple casts so we disallow cases like this (even in cases where you can convert such as when `c : (int) | int = (9)` which would allow it to be converted to a tuple) and force you to be explicit about it like `swap((c), a)` (though actually this introduces a temporary maybe tuple!).  If you want to instead do a cast but without the temporary you would have to add `c = (c)` before the call.

## Syntax oddities around blocks

There are some minor syntax oddities around blocks that benefit you.  This is really the only place in the syntax that is a tad odd, it also isn't unusual to have these kind of oddities for example both Rust and Jai do similar things.

The general rule is that ONLY functions can avoid having a `;` and any block that isn't used as an expression.

> The reasons we are leniant on functions is that they only have one operator that can be used with them (the folding one) otherwise you have to either call it or pass it as an argument (by using folding op! or just manually) this makes it very cheap to check for the `|>` operator and if you mistyped and forgot it you'll get an error of invalid arguments (since you'll have one less!) so its really quite safe.  On the other hand integers wouldn't be safe cause `1 \n + a()` could be mistyped as `1 \n a()` and that holds a very different meaning and won't necessarily give you an error!!

```python
# note: `if y w else z` is also valid here the parenthesis are just to make
#       this more readable.
# 1a)
x := if (y) 10 else 100;

# 1b)
x : int;
if (y) {
    x = 10;
} else {
    x = 100;
}

# 2a)
x := if y {
    # you can just just a single `=` in cases like this
    = w;
} else {
    = z;
};

# 2b)
x : int;
if y {
    x = w;
} else {
    x = z;
}

# 3a) blocks
x := {
    # same here as in 2a)
    = if (p) 40 else 100;
};

# 3b)
x : int;
if (p) {
    x = 40;
} else {
    x = 100;
}

# 4a) generators
odd := for i in 0.. {
    if i % 2 != 0 { yield continue i; }
};

# 4b)
# we would need a function...
GetOdds :: () {
    # just some manual examples that work
    for i in 5.. {
        if i % 2 != 0 {
            yield return i;
        }
    }
}

# 5a) non generator creations
# note: this will give some warning about an infinite range with no generators
#       if you don't include the @infty_range at the beginning
first_10_evens := @warn.infty_range() <| for i in 0.. {
    # you can refer to variables like this since it builds it up as you go
    # you could even loop through what you have created so far :O
    if first_10_evens.len == 10 {
        break;
    }
    if i % 2 == 0 {
        continue i;
    }
};

# 5b) a slightly better version
it := 0..;
first_10_evens := while first_10_evens.len < 10 {
    i = it.next();
    if i % 2 == 0 {
        continue i;
    }
};

# which is really just 5c)
it := 0..;
first_10_evens := List[int](capacity=10);
while first_10_evens.len < 10 {
    i = it.next();
    if i % 2 == 0 {
        first_10_evens.append(i);
    }
}

# or
first_10_evens := List[int](capacity=10);
# just giving it some upper bound so I don't have to pipe it through a suppress
# else I would have to put the `;` at the end since it would be an expr
for i in 0..int.max_value {
    if (first_10_evens.len == 10) break;
    if (i % 2 == 0) first_10_evens.append(i);
}
```
