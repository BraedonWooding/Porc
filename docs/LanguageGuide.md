# Introduction

Porc is a fully open sourced dynamic programming language that focuses on fixing the issues of other dynamic languages!

It is meant to be both a sucessor (of sorts) to shell languages (i.e. bash) while also functioning as more general purpose (i.e. python) while also being performant enough to compete with performant dynamic languages (i.e. LUA).

# NOTE:

A lot of this is pseudo outdated, I just wanted to get ideas onto 'paper'.  For example struct syntax changed to `color = (r: int, g: int, b: int) :: { /* ... */ }` from `color = struct { r: int; g: int; b: int; /* ... */ }`

## Hello World

`hello.porc`

```rust
println("Hello World");
```

But you can also wrap in it in a main if you want.

```rust
main = fn () => {
    println("Hello World");
}
```

```bash
$ porc run hello.porc

Hello World
```

## Comments

`comments.porc`

```rust
// we support single line comments
/* and
   multi-line
   comments
*/
```

## Variables/Basic Types

This isn't going to discuss complex combinatorial types just simple ones.

`var_and_types.porc`

```rust
x = 2;
y = "Hello World"
{
    x = 9;
    x += 2;
    println(x); // 11
}
println(x); // 2
z: int;
// below are all compile errors
// println(z);
// z = "Hello";
// this is fine
z: str;
z = "Hello";
```

Output;

```bash
$ porc run var_and_types.porc

11
2
```

## Some more complex types

The type system is quite expressive, let me demonstrate it through functions;

`types_extra.porc`

```rust
fib = fn (n) => 1 if n <= 1 else fib(n - 1) + fib(n - 2);
println(fib(4));
// compile error below:
// println(fib("Str"));
// however this is fine but produces maybe unexpected output
println(fib(-1));
// or
println(fib(10.355));
// so we can give it a type (this is normally defined under int.positive but I'll
// recreate it here for example)
// you could also just use an uint but let's just use int for the sake of it
IntPositive = (data: int) :: {
    new = fn (data) => {
        // this allows it to pick some errors up at compile time rather than
        // just at runtime.
        @comp_assert(data >= 0);
        // last statement always returned unless ends with `;`
        // this does mean that in things like `add_no_return = (a, b) => a + b;;`
        // you need the double `;` to not return you could also just write it as
        // `add_no_return = (a, b)void => a + b;`
        (data)
    };

    // this enables the cast `IntPositive(2)`
    @impl_cast_from(int) <- fn (data) => IntPositive.new(data);
    // and `int(IntPositive.new(2))`
    @impl_cast_to(int) <- fn (self) => self.data;
}

// return type is after the ()
fib = fn (n: IntPositive) IntPositive => 1 if n <= 1 else fib(n - 1) + fib(n - 2);

// You can go further using the implements syntax `^` which just makes sure that
// such a function exists in which it can call it like such.
// You can have pseudo recursive type definitions as long as the recursion
// occurs within any implement 'section' of the type
animal_noise = fn (animal: any ^ fn (animal)str) => (fn(any)str)(animal);

// 
make_noise: fn(any)str;
animal_noise = fn (animal: any ^ make_noise) => animal->make_noise()
```

## Deep End

I'm just going to show a pretty advanced example then break it down.

`deep_end.porc`

```rust
x = 2;
println(x);
x = 500.3995;
println(x);
x += 50;
println(x);
y = "Hello World"
println(y + "!");

// this isn't a pointer dereference, this will be explained elsewhere
// it is rather equivalent to `println(str.split(y, " "));`
println(y->Str.split(" "));

// can either be an int (from 0 to 255) or a normalized flt (0 to 1)
// we pass in information to the normalized so it knows how to normalize
ColorComponent = int.bounded_range(0, 255, 1) | flt.normalized(type: int, factor: 255);
ColorTuple = (ColorComponent, ColorComponent, ColorComponent)
Color = struct {
    r: ColorComponent, g: ColorComponent, b: ColorComponent;

    // implicit types
    @constructor() <- (self, r, g, b) => {
        self.r = r;
        self.g = g;
        self.b = b;
    };

    normalized = (self) => {
        return Color.new(normalize(self.r), normalize(self.g), normalize(self.b));
    };

    rgb = (self) => {
        // reverses a normalized reaction
        return Color.new(un_normalize(self.r), un_normalize(self.g), un_normalize(self.b));
    };

    // the <- does the reverse of -> i.e. `a(b) <- c` does `a(b, c)`
    @impl_to(ColorTuple) <- (self) => {
        return (self.r, self.g, self.b);
    };

    @impl_from(ColorTuple) <- (r, g, b) => {
        return Color.new(r, g, b);
    };

    @impl_to(str) <- (self) => {
        // converting from tuples to strings are already defined so instead
        // of doing something like `format("\{self.r\}, \{self.g\}, \{self.b\}");`
        // we can just do this
        return str(ColorTuple(self))
    };

    @impl_from(str) <- (string) => {
        return Color(ColorTuple(string));
    };
};

orange = Color("255, 165, 0");
println(orange);
orange.r = 1.0;
println(orange);
println(orange->Color.normalized());
println(orange->Color.rgb());
```

With the output;

```bash
$ porc run types.porc

2
500.3995
550.3995
Hello World!
Hello
World
255, 165, 0
1.0, 165, 0
1.0, 0.65*, 0
255, 165, 0
```

> * note: I've just done some rounding for readability

There is a LOT to unpack here!!!!  One of the things you may notice is that functions are first class citizens this is actually an incorrect presumption since everything is a first class citizen this is because there are no such things as 'types' or 'variables' rather just constraints and views placed on data.  Think of it this way;

If I have the following unsigned byte `01000000` this can be interpreted in various ways for example the number 64 or the character '@' or perhaps the boolean value of `true`, or perhaps a memory address.  This can be extended even further for longer sequences (maybe its a float, or a class or a struct) the fact is that memory doesn't have 'types' and often it is easier to conceptialise code when you remove types in this case you just have various ways to view the data.

HOWEVER this doesn't mean we 'bitcast' the data the data is converted appropriately this is represented often by a 'base' representation however since we have abstracted types away we don't necessarily have to have a single 'one' base representation for example in the above example ColorComponents can either be a normalized float between 0 and 1 or an integer between 0 and 255, so while there is still a base format for the type it doesn't have one base format but rather two!

This leads to awesome things such as the ability to have each component by individual to the others for example the red component in the above example is converted to a normalized variant where as the other is the integer version (you can of course state that as an invariant they all must remain the same), another cool result is the fact that conversions become explicit by common sense, if nothing has a type you have to detail how you want to view it!  This improves readability.

A somewhat consequence of this is that there are no such things as 'member functions' of a type one of the reasons why you have to write `orange->Color.normalized()`.  But why can we access member variables such as `r`?  Well actually you can treat a variable (really just a label) as being of its base type otherwise in this example you would probably have to do something like `Color.r(orange, 1.0)` (or `orange->Color.r(1.0)`).  This is just to improve accessibility of classes.

## Classes/Structs cont

Now let's say you have something like this;

`classes_cont.porc`

```rust
// Presuming the colour class that was defined before
red = Color.new(255, 0, 0);
other_red = (255, 0, 0);
ref_red = red;
// the above two may seem identical and are for the following check
println(red == other_red); // memory comparison
println(cmp.eql_mem(red, other_red));
println(cmp.eql_ref(red, other_red)); // checks if they point to the same place
println(cmp.eql_ref(red, ref_red));
```

Running...

```bash
$ porc run classes_cont.porc

true
true
false
true
```

### Side Note: A better color class

Color could actually be defined like this;

```rust
ColorComponent = int.bounded_range(0, 255, 1) | flt.normalized(type: int, factor: 255);
ColorTuple = (ColorComponent, ColorComponent, ColorComponent)
Color = struct {
    r: ColorComponent, g: ColorComponent, b: ColorComponent;

    // ctor is the same

    @impl_tuple(ColorTuple, Color.new, r, g, b);
    @impl_middleman(str, ColorTuple, Color);

    normalized = (self) => return Color(normalize(ColorTuple(self)));
    rgb = (self) => Color(un_normalize(ColorTuple(self)));
};
```

In `@impl_tuple` you just give the tuple type, the constructor for your object, and the tuple members to be referenced like `self.r, self.g, self.b` or to be passed in, this allows you to have multiple other members that are handled seperately.

In `@impl_middleman(a, b, c)` it just writes out `a(b(self))` and `c(b(string))` basically just using a middle man to convert between them.

And as you can see we can use those to make the normalized and rgb functions.  We have just cut down the boiler plate code from ~20 lines to just 4.

## Functions

`functions.porc`

```rust
@ignore(duplicates)

// the following are all identical
add = (a, b) => a + b;
add = (a, b) => {
    return a + b;
};
// last statement without a semi-colon is 'auto returned'
add = (a, b) => {
    a + b
};
// and this will work for all the various types of return
// `any ^ ops.add(a, b)` can be read as `any that implements ops.add where ops.add(a, b) is valid`
add = (a: any ^ ops.add(a, b), b: any ^ ops.add(a, b)) ops.add(a, b) => a + b;
// ~a is the complement of a which converts `^` to `|` (effectively)
// so in this case it is saying `any | ops.add(a, b)` and `any |` is an invalid constraint
// so it is trimmed so it just becomes `ops.add(a, b)`
add = (a: any ^ ops.add(a, b), b: a) ~a => a + b;
```

> The evaluation of a label can either be as a type evaluation or as a value evaluation if the evaluation is to partake during compilation it is a type evaluation else it is a value.

## Macros

Macros are built into the type system, there is only one rule for macros;

> The expressions passed into a macro must consist of valid tokens

`macro_examples.porc`

```rust
// you may have seen this
// the `*` is a valid token of course so this is valid
@import("BraedonWooding/cmd", *);
// this is valid (the extra `.` is still a token, this forms two tokens a number and `.`)
@valid_macro1(5.50., cat, (other, cool))
// this is however not valid
@invalid_macro("Cool)
```

You define a macro like;

```rust
@define_macro(add, a, b, {
    a + b
});
```

A few things to note about macro definitions:

* There are literally no types, it purely is a text replacement tool
* Everything is scoped correctly so you don't have to do `((a) + (b))` that is 'automatic'
* The block you pass **is not** evaluated so make sure you have tests that use the macro!

## Case Study: Rust's Result Type

We handle errors practically identically to Rust, our error type however can be defined in a few ways!

Typically functions have less crazy error handling requiring less `?` for example you can use `@global(abort_on_err)` which means that whenever a function returns an std.err it will rather abort if it isn't caught, this may be the exact type of error handling you want or maybe you want more control, in that case you can add the error handling where you want it or can just not use that global and do it everywhere.

### Using the type system

```rust
// since types don't really have a runtime rep
// in reality this isn't needed I just included this to follow the rust syntax.
@define_macro(Result, ok, err, { ok | err });

parse_bool = (string: str) Result(bool, str) => {
    if (string == "true")       return true;
    else if (string == "false") return false;
    else                        return "Invalid Type"
};

main = () => {
    res = parse_bool("true");
    // if we know it is valid
    // we pass the type as this is just a normal macro that just applies
    // some proper conversions
    println(@unwrap(res, bool)):
    // we could also check it like this
    if (@type(res) == bool) {
        println(res);
    }
    // more idiomatically
    match (res) {
        // note: res is still bool | str here it just that println will accept both
        // if we wanted to force it to be bool we could do `bool(val) => println(val);`
        bool => println(res);
        str => println("Error \{res\}");
    }
    // note in rust there is the `?` operator we don't have this operator yet
    // however the following is often sufficient (also look at the std.err class for nicer stuff)
    if (err: str = res) return err;
};
```

Yes in this case as you can see Result is just nice readability candy!  We have a special error class as well if you want a bunch of extra functionality (not shown here just has nicer printing and supports things like enums and error codes);

```rust
parse_bool = (string: str) Result(bool, std.err) => {
    if (string == "true")       return true;
    else if (string == "false") return false;
    else                        return std.err("Invalid Type");
};

// this gives you stuff like
main = () => {
    res = parse_bool("true");
    if (res->std.err.is_err()) return err;
    // and you can use the `->` with the `@try` macro to simulate the `?`
    // this only works if one of the types of `|` is std.err
    value = @try(res);
    // or
    handle_value(res->@try())
    // for example a chaining
    err_function(data)->@try()->output_to_user();
    // is the same as
    output_to_user(@try(err_function(data)));
};
```

And as you can probably guess there is just a `@impl_from(str)` defined in std.err to make it work for strings!

### Using Typed Structs

This way is practically identical to the way Rust does things (in usage);  I would suggest this way since it uses a few weird tricks but it's a decent example of the powerful types.

```rust
Result = struct {
    // void has no data storage except the type
    value_type: void ^ any;
    err_type: void ^ any;

    // no value enums they aren't needed we do have integral enums (C++ scoped styled ones)
    // ~ evaluates the types then swaps `|` for `^` (and vice versa)
    // so for example if value_type is 'int' and err_type is 'std.err'
    // then it'll become ~(void ^ int) | ~(void ^ std.err) => (void | int) | (void | std.err)
    // `void | x` really just means `x` since when evaluated you can't have a type instantiated without data
    // (that is void can only be used like `void ^ something`) this means that it becomes just `int | std.err`
    data = ~value_type | ~err_type;

    @constructor() <- (self, data) => {
        self.data = data;
    };

    // the use of @type here annoys me!!  One of the reasons why I don't like this impl
    is_err(self) => return self.err_type == @type(data);
    is_val(self) => return self.value_type == @type(data);
}

// just to make it look like rust
Ok = (data) => Result.new(data);
Err = (err) => Result.new(err);

parse_bool = (string: str) Result => {
    if (string == "true")       return Ok(true);
    else if (string == "false") return Ok(false);
    else                        return Err(std.err("Invalid Type"));
};

main = () => {
    res = parse_bool("true");
    // akin to rust
    match (res) {
        Ok(val) => println(val);
        Err(err) => println("Error \{err\}");
    }
    // or this works too, and the above will most likely just become this
    match (res.data) {
        val if Result.is_val(res) => println(val);
        err if Result.is_err(res) => println("Error \{err\}");
    }
    // heck this also works
    match (res.value_type) {
        // this is compile time needed so its type referred
        // however getting it converted will require a unwrap most likely
        res.data => println(res.data);
        // else just covers the 'default' case
        else => println("Error \{res.data\}");
    }
}
```

The `void ^ any` 'trick' is quite useful to make generic data structures that maintain just a single type for example a very simple 'Vector' (C++ dynamic array) implementation;

## Case Study Vector Implementation

### Untyped Implementation

`vector.porc`

```rust
Vector = (data: [any], cur_len: int.positive) ^ ops.index() ^ ops.add() ^ Array.len() :: {
    new = (initial_size = 0) => (Array.new(initial_size), initial_size);

    at = @bind(Vector, ops.index) <- (self: Vector, index: int.postiive) any | std.err => {
        if (self.cur_len <= index) return std.err("Out of range");
        return self.data[index];
    };

    push = (self, values...) => {
        if len(self.data) < values.len + self.cur_len {
            self.data = Array.resize(data, self.cur_len + values.len);
        }

        for (value in values) data[cur_len++] = value;
    };

    len = @bind(Vector, Array.len) <- (self: Vector) int.positive => self.cur_len;

    add = @bind(Vector, ops.add) <- (a: Vector, b: Vector) Vector => {
        return new(len(a) + len(b))->push(a.data...)->push(b.data...);
    }

    @conv_from([any]) <- (arr) => (arr, Array.len(arr));
    @conv_to([any]) <- (self: Vector) => self.data[0..self.cur_len];
};

main = () => {
    numbers = Vector.new();
    numbers->Vector.push(10);
    numbers->Vector.push(100);
    numbers->Vector.push("WOW");
    x = @try(numbers[0]);
    println(x);       // 10
    // note: ^ is still type 'any' you will need some kind of match or
    // unsafe cast to get it to int
    println(numbers); // 10, 100, "WOW"
    // we could also say
    numbers = [1, 2, 3, 5.4];
    println(numbers); // 1, 2, 3, 5.4
    numbers = Vector(numbers);
    numbers->Vector.push(10);
    println(numbers); // [1, 2, 3, 5.4, 10], 5
    // we could add a @conv_to(str) method if we wanted better control over the look
    // as this vector does support arrays biggert han the cur_len which would contain junk data
    // in the output.  Something like;
    // `@conv_to(str) <- (self: Vector) => str(self.data[0..self.cur_len]);`
};
```

## Typed Implementation

```rust
Vector = () => {

}

Vector = (data: [any], cur_len: int.positive) ^ ops.index() ^ ops.add() ^ Array.len() :: {
    new = (initial_size = 0) => (Array.new(initial_size), initial_size);

    at = @bind(Vector, ops.index) <- (self: Vector, index: int.postiive) any | std.err => {
        if (self.cur_len <= index) return std.err("Out of range");
        return self.data[index];
    };

    push = (self, values...) => {
        if len(self.data) < values.len + self.cur_len {
            self.data = Array.resize(data, self.cur_len + values.len);
        }

        for (value in values) data[cur_len++] = value;
    };

    len = @bind(Vector, Array.len) <- (self: Vector) int.positive => self.cur_len;

    add = @bind(Vector, ops.add) <- (a: Vector, b: Vector) Vector => {
        return new(len(a) + len(b))->push(a.data...)->push(b.data...);
    }

    @conv_from([any]) <- (arr) => (arr, Array.len(arr));
    @conv_to([any]) <- (self: Vector) => self.data[0..self.cur_len];
};

main = () => {
    numbers: Vector.typed(int) = Vector.new();
    numbers->Vector.push(10);
    numbers->Vector.push(100);
    x = @try(numbers[0]);
    println(x);       // 10
    // ^^ is an int
    // you couldn't do `numbers->Vector.push("boo");`
    println(numbers); // 10, 100
    // we could also say
    numbers = [1, 2, 3];
    println(numbers); // 1, 2, 3
    numbers = Vector(numbers);
    numbers->Vector.push(10);
    println(numbers); // [1, 2, 3, 10], 4
    // we could add a @conv_to(str) method if we wanted better control over the look
    // as this vector does support arrays biggert han the cur_len which would contain junk data
    // in the output.  Something like;
    // `@conv_to(str) <- (self: Vector) => str(self.data[0..self.cur_len]);`
};
```

## Some cool things

This should hopefully inspire some confidence in the type system!

### println

You may have noticed that println can basically take anything you can do more of a formatted print like; `println("\{a\} + \{b\} = \{c\}")` or you can print lists like `println(list)` or literally almost anything!  How do we do this?

Well println is literally just this (keep in mind we don't actually use libc in our implementation but its a good example of really what is happening as well as the fact that the actual definition has a few more little attributes and details to fix common mistakes).

```rust
println = (contents: any ^ str) => {
    puts(str(contents));
    putchar('\n');
}
```

So in reality println is literally just a `puts` that then also prints out the newline character...?  How does this work??  Well the magic is in `any ^ str` this ensures that the given object is convertible to string and our type system makes this extremely easy with `@impl_to` and `@impl_from`.  Also any tuple/list is automatically convertible if the objects inside it are convertible (if they aren't it'll just print out the memory address).

### An even better color

A better color that could also support hex would be;

```rust
ColorComponent = int.bounded_range(0, 255, 1) | flt.normalized(type: int, factor: 255);
ColorTuple = (ColorComponent, ColorComponent, ColorComponent)
Color :: struct {
    // hexadecimal is 24 long and so to pack space we can set this to emulate that
    data: ColorTuple | int.width(24);

    // implicit types
    new_rgb = (r, g, b) => {
        return Color {
            .data = (r, g, b);
        };
    };

    new_hex = (hex) => {
        return Color {
            .data = hex;
        };
    }

    @impl_to(ColorTuple) <- (self) => {
        return (@type(self.data) == int.width(24))
               ? (0xFF0000 & self.data, 0x00FF00 & self.data, 0x0000FF & self.data);
               : self.data;
    };

    @impl_from(ColorTuple) <- (self, tuple) => {
        return new_rgb(@unpack(tuple));
    };

    @impl_to(int.width(24)) <- (self) => {
        return (@type(self.data) == ColorTuple)
               ? self.data[0] << 16 | self.data[1] << 8 | self.data[2];
               : self.data;
    };

    @impl_from(int.width(24)) <- (self, hex) => {
        return new_hex(hex);
    };

    @impl_to(str) <- (self) => {
        return (@type(self.data) == ColorTuple)
               ? str(self.data)
               : str.hex(self.data);
    };

    @impl_from(str) <- (string) => {
        return Color((@type(self.data) == ColorTuple)
               ? ColorTuple(string)
               : int.width(24)(string);
    };

    normalized = (self) => return Color(normalize(ColorTuple(self)));
    rgb = (self) => Color(un_normalize(ColorTuple(self)));
    hex = (self) => Color(to_hex(self));
};
```

Now you can avoid the typechecks by effectively introducing a converting concept however that is a slightly more complicated topic so I've omitted that here, effectively you are just defining a class with the contents of the hex code with all these `to_x` methods in it; basically doing something like this;

```rust
ColorComponent = int.bounded_range(0, 255, 1) | flt.normalized(type: int, factor: 255);
ColorTuple = (ColorComponent, ColorComponent, ColorComponent)
HexCode :: struct {
    data: int.width(24);

    new = (data) => {
        return HexCode {
            .data = data;
        };
    };

    @impl_to(ColorTuple) <- (self) => {
        return (0xFF0000 & self.data, 0x00FF00 & self.data, 0x0000FF & self.data);
    };

    @impl_from(ColorTuple) <- (self, tuple) => {
        return new(self.data[0] << 16 | self.data[1] << 8 | self.data[2]);
    };

    // there is even a way that you can simplify these since they just use
    // a middleman effectively
    @impl_to(str) <- (self) => {
        return str.hex(self.data);
    };

    @impl_from(str) <- (string) => {
        return new(int.width(24)(string));
    };

    to_int, from_int = @impl_member(HexCode, int.width(24), data);
};

Color :: struct {
    // hexadecimal is 24 long and so to pack space we can set this to emulate that
    data: ColorTuple | HexCode;

    // implicit types
    new_rgb = (r, g, b) => {
        return Color {
            .data = (r, g, b);
        };
    };

    new_hex = (hex) => {
        return Color {
            .data = hex;
        };
    }

    @impl_member(Color, ColorTuple, data);
    @impl_member(Color, HexCode, data);

    normalized = (self) => return Color(normalize(ColorTuple(self)));
    rgb = (self) => Color(un_normalize(ColorTuple(self)));
    hex = (self) => Color(to_hex(self));
};
```

This effectively removes the if checks by bringing them out into the type system, `@impl_member` is actually extremely easy to replicate it is just;

```rust
@define_macro(impl_member, main_type, sub_type, member, {
    fn (self) => {
        return sub_type(self.member);
    },
    fn (data) => {
        return main_type {
            .member = data;
        };
    }
});
```

## Structs

```rust
ColorComponent = int.bounded_range(0, 255, 1) | flt.normalized(type: int, factor: 255);
Color = (r: ColorComponent, g: ColorComponent, b: ColorComponent) :: {
    new = fn (r, g, b) Color => {
        return (r, g, b);
    }

    normalized = (self: Color) Color => return Color(flt.normalize(self));
    rgb = (self: Color) => return Color(flt.normalize_reverse(self));
}

main = () => {
    red = Color("255, 0, 0");
    red.r = 1.0; // this is fine since the tuple is defined with member names
    red[0] = 1.0; // also fine
    println(red); // 1.0, 0, 0
    c = red;
    c.g = 255;
    println(c); // 1.0, 255, 0
    println(Color::normalized(c)); // 1.0, 1.0, 0
    println(Color::rgb(c)); // 255, 255, 0
    // same as saying
    println(c->Color::normalized());
    println(c->Color::rgb());
};
```

## Memory

Porc has an interesting mix of memory management styles, typically you will use the auto memory model which will garbage collect large chunks of memory or when it runs out of it's static memory, basically it looks something like this;

```rust
request_memory(amount) {
    if small && space_left then {
        return allocate_chunk(amount);
    } else {
        mem = malloc(amount);
        gc_track(mem);
        return mem;
    }
};
```

Effectively this is just one of our memory models (currently the only one we support) later on we will support others like manual memory management (for more real time systems) and more of a mix (maybe also a full gc collected ones).

This means that you can improve the speed of your application by chunking down your objects into smaller memory amounts (pratically any non-trivial array will be malloc'd which is usual anyways for C applications), and by not keeping memory around for ages; this is normal for GC languages however the impact is greater in Porc.  If you want to keep long living short memory then you may want to ask it to be gc'd so you don't use up the space (if it is significant enough to impact the short lived memory segments) this can be done with a `@long_lived_memory()` call on any object creation function. i.e. `@long_lived_memory(Color.new(255, 0, 0))`, this will find the allocations that have the possibility of being long lived and will GC track them.

This happens automatically for some things (i.e. the attribute/macro/function is applied inside the new) for example files or streams.
