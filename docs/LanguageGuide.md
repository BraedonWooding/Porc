# Language Guide - Porc

This is just a language guide for Porc, it is relatively basic for now.

## Hello World

`hello.porc`

```rust
println("Hello World");
```

But you can also wrap in it in a main if you want.

```rust
main = fn () => {
    println("Hello World");
};
```

```bash
$ porc run hello.porc

Hello World
```

## Functions/Variables

`functions_variables.porc`

```rust
// starting at 1
// 1, 1, 2, 3, 5, 8, ...
// 'where' guard allows you to define constraints
// you can also use the type system (explained later)
// you may prefer this to types so I felt I should include it here
// you need to have the `{` after where by the way.
fib = fn (n) => where n > 0 && n |> @is(int) {
    if (n <= 1) return 1;
    return fib(n - 1) + fib(n - 2);
};
// which is identical to
fib = fn (n) => {
    return
        if (n <= 1) 1
        else fib(n - 1) + fib(n - 2);
};
// identical to
fib = fn (n) => { return n <= 1 ? 1 : fib(n - 1) + fib(n - 2); }
// which is again identical to just
// (Note: the lack of semi-colons to signify a return)
fib = fn (n) => {
    if (n <= 1) 1
    else fib(n - 1) + fib(n - 2)
};
// and as a final way
fib = fn (n) => n <= 1 ? 1 : fib(n - 1) + fib(n - 2);

// you could for example bind it like
fib_two = @bind(fib, 2);
println(fib_two()); // 2
// binding is identical to
// however it also handles the cases of partial better
// (you could of course just hand write those cases just as easily)
fib_two = fn () => fib(2);

x = "Hello World";
println(x.split(" ")); // ["Hello", "World"]
```

## How objects are represented

First let's look at how objects are defined

`object.porc`

```rust
// to allow disabling type checks later on
@incl(comp)

// using the classic color example
Color = (r: int, g: int, b: int) :: {
    // automatically you can cast between tuple lists and this class
    // note: this automatically also includes a to string conversion through
    // the tuple set but you can override it

    // self is a special parameter (not including it makes the function 'static')
    normalized = fn (self) => {
        return (self.r / 255.0, self.g / 255.0, self.b / 255.0);
    }
}

// note: this is a cast and is identical to Color((100, 255, 0))
// you can also do casts like @cast((100, 255, 0), Color)
// tuples can be elided in cases like this because there is no conflicting constructor
red = Color(100, 255, 0);
println(red); // r: 100, g: 255, b: 0
println(red.normalized); // 0.3922, 1, 0
println(Color.normalized(red));
// Note: it can cast here to Color implicitly
println(Color.normalized((0, 255, 255))); // 0, 1, 1
@comp.disable_type_checks()
// but this will fail
// (NOTE: disabling type checks so this will actually compile but fail at runtime
// and we can just print the error and continue, this is quite bad code to write
// in practice but it is good to show you the hand holding the type system gives you)
@try(fn () => {
println(Color.normalized((0.0, true, "Wow"))); // Runtime Error: Can't convert between (flt, bool, str) and (int, int, int).
}, fn (err) => println("Runtime Error: ${err}"));
@comp.enable_type_checks()
```

Objects are just binded tuples this has a few interesting 'results';

- Inheritance doesn't 'exist' in the same way that is common to OOP scripting languages meaning memory is easier to manage and is more efficient (discussed below this list)
- We can express classes as just tuples in terms of memory (static tuples that is)
  - This not only makes them more memory efficient but more performant
- Duck typing can't exist (we have a proper type system though)
- Virtual calls are grossly more efficient
- We can't JIT a lot of our code (well rather it is a lot hard to do it)
  - This is mainly also due to the type system (covered a bit lower in this guide)

## Inheritance

Basically inheritance is a large part of the type system rather than being an object driven thing.  There are two kinds of inheritance that can happen.

### Abstract/Interface Inheritance

In this case the data and methods are lifted up into the derived class and no virtual calls have to occur except in the case where you are viewing it as the abstract.

`abstract.porc`

```rust
// the where just means that you can create an animal and that it shouldn't generate
// the conversion methods.
Animal = (name: str, age: int, type: str) :: where @abstract {
    get_noise = fn (self) str;

    // can provide an implementation
    can_fly = fn (self) bool => false;
    can_jump = fn (self) bool => false;
};

Jump = (void) :: where @abstract {
    can_jump = fn (self) bool => true;
    jump = fn (self) => println("I am jumping!");
};

Fly = (void) :: where @abstract {
    can_fly = fn (self) bool => true;
    fly = fn (self) => println("I am flying!");
};

// you have to define the tuple members (you can change the order from the inherited class)
// I've not included type here because I'll define it inside
Bird = (name: str, age: int) :: Animal, Fly {
    type = "Bird";
    get_noise = fn (self) str => "Birp";
};

Pig = (name: str, age: int) :: Animal, Jump, Fly {
    type = "It's a flying Pig!";
    get_noise = fn (self) str => "Am I Porc?";
    // overriding works (because all functions are technically represented in the derived class anyways)
    fly = fn (self) => println("Should I be able to fly??");
};

// feel free to include more
Dog = (name: str, age: int, type: str, noise: str) :: Animal, Jump {
    get_noise = fn (self) str => noise;
};

// `^` explained under type system just means implements
make_jump = fn (x: any ^ Jump) => x.jump();
make_fly = fn (x: any ^ Fly) => x.fly();

amelia = Dog("Amelia", 7, "Pug", "Roof");
println(amelia.get_noise()); // Roof
amelia.jump(); // I am jumping!
make_jump(amelia); // I am jumping!

oinkers = Pig("Oinkers", 1);
oinkers.fly(); // Should I be able to fly??
make_fly(oinkers); // Should I be able to fly??
// You can try to call the base class's method
// This is reliant on the base class existing in the IR
// It may not, in that case it is a compile time error (not runtime don't you worry!)
// It won't exist if you pass the flag `--trim-base-classes=true` (the `=true` is optional)
// (Which may be on by default if compiling on a higher O level so you may have
// to add `--trim-base-classes=false` to override that)
@base_call(oinkers, Fly, fly); // I am flying!
```

A few notes about inheritance;

- You can't inherit and state a where statement meaning you can't be both abstract (/ interface) and inherit from a class
  - This may change in the future.

## Type System

Why explain the type system so late?  Well I feel it is necessary to explain it after you understand functions, objects and why we have objects a certain way.  Since the type system was built as a way to address the needs of duck typing while also making the typing strict (kinda contradictory but you will see).  So the type system was built on two premises;

- Only need to state types when absolutely necessary (that is there is no way to accurately infer)
  - Example would be the make_jump function in the object example or a function with only a definition.
- As expressive as dynamic and duck typing but as strict as static.
  - That is offload a lot of the unit testing that dynamic languages need (just to check types) into the type system while simultaneously not removing any of the productivity you have in those languages.
- And finally while not a premise it was built on but is something we need to pay attention to is performance!

`types.porc`

```rust
// side-note: this won't compile since we are going to be having tons of type errors
// just to show you where the type errors are if you want to compile it just comment
// out the lines that have a `*ERR*` comment above them

// you use `:` to state the type
// primitive types are; str, int, uint, char, flt, bool
x: str;
// *ERR* variable is undefined
println(x);
x = "Hello World";
println(x); // "Hello World"
// *ERR* Invalid Type
x = 3;

// you can redefine
x: int;
// *ERR* variable is undefined
println(x);
x = 3;
println(x): // 3
// *ERR* Invalid Type
x = "Hello World"

// you can use `|` to state a variant
// you can also give a value when defining type of course
x: int | str = 9;
x = "hello world"

// you use `^` to state something implements some function
// you can also use this to state that something is convertible to some type
// i.e.
Color = (r: int, g: int, b: int) :: {
    normalized = fn (self) => { (self.r / 255.0, self.g / 255.0, self.b / 255.0) };

    @conv_to(str) <- fn (self)str => return "RGB: ${self.r}/${self.g}/${self.b}";
};

print_color_no_cast = fn (color: any ^ Color ^ str) => println(color);
print_color_cast = fn (color: any ^ Color ^ str) => println(Color(color));

red = Color(255, 0, 0);
print_color_no_cast(red); // RGB: 255/0/0
print_color_cast(red); // RGB: 255/0/0
red_tuple = (255, 0, 0);
print_color_no_cast(red_tuple); // Tuple(r: 255, g: 0, b: 0)
print_color_cast(red_tuple); // RGB: 255/0/0
```

> Note: you don't have to include the types for print_color_cast/no_cast they are purely just for you to see the syntax

Effectively what is happening in the color example is that println is defined just as `println = fn (to_print: str);` this means that it will just cast it to a string and that it won't think of casting it to a Color since you didn't ask it to!  This means that you can get type ducking behaviour really easily just as you would before by defining types such as; `fn (x: any ^ fn (int) str)` or similar.

So you get the best of both worlds!  You can make it type ducked (effectively) by not giving types or you can give types and make it interface based!

A few notes about types;

- They aren't first class citizens and have to be resolved at compile time.
- You can pass them around to functions however to achieve generics that way if you want for example;

`generics.porc`

```rust
// you can just use `any` to implement generics i.e.
// when you call a function in the context of types it will either grab the return type
// or in the context of `^` it will validate that the function is valid (i.e. types match constraints)
add = (a: any ^ ops.add(a, b), b: any ^ ops.add(a, b)) ops.add(a, b) => a + b;
// you could also write this (the type system will just add what is above)
add = (a, b) => return a + b;
// you could use `where` to also add the constraints
add = (a, b) => where @type_valid(ops.add(a, b)) { a + b };

// you could also use a type one I guess
// note: that `{ a + b }` is identical to `return a + b`
// you could also just say `a + b` if you explictly state the type
add = (a_type: @type, b_type: @type, a: a_type, b: b_type) => return a + b;
```
