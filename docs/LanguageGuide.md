# Language Guide

Welcome to Porc!  Let's begin with the standard rite of passage; `hello world!`

```c
println("Hello World!");
```

This doesn't really show anything about Porc so a nicer example would be perhaps 

## Syntax oddities around blocks

There are some minor syntax oddities around blocks that benefit you.  This is really the only place in the syntax that is a tad odd, it also isn't unusual to have these kind of oddities for example both Rust and Jai do similar things.

The general rule is that ONLY functions can avoid having a `;` and any block that isn't used as an expression.

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
first_10_evens := List[int](10);
while first_10_evens.len < 10 {
    i = it.next();
    if i % 2 == 0 {
        first_10_evens.append(i);
    }
}
# or
it := 0..;
first_10_evens := List[int](10);
# just giving it some upper bound so I don't have to pipe it
# else I would have to put the `;` at the end since it would be an expr
for i in 0..20 {
    if first_10_evens.len == 10 {
        break;
    }
    i = it.next();
    if i % 2 == 0 {
        first_10_evens.append(i);
    }
}
```
