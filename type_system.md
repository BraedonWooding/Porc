# Porc's Typing

Porc's type system is meant to allow you to code in a dynamic way while having everything statically defined.  Of course you can add types (and it is recommended to do so for function definitions for readability purposes) but for a quick script you don't have to worry about it!

## Example

```rust
// note: n is 0 indexed
// no need for return in this case
fib = fn (n) => n <= 1 ? 1 : fib(n - 1) + fib(n - 2);
```

> What is the type of `n`?

It isn't int and actually nothing mandates that it has to be defined as a primative.  It actually is defined as;

`n: any ^ ops.sub(n, n(int) ? n(int) : int) ^ ops.lte(n, n(int) ?? int)`.

Now you may ask why it isn't defined in terms of `ops.add`?  Well since fib() always returns an int `n` doesn't necessarily have to implement the addition operand.  So what can we call fib with?

- `fib(1) = fib(0) = 1`
- `fib(4) = 5`
- `fib(3.9) = 5`
- `fib(3.3) = 5`

Now the fact you can pass a float may surprise you!  But in reality since these operations are defined well on both it is fine.  To break down the type...

- `any` just means any type, similar to `Object` in Java/C# all objects are 'any'
- `^` just means implements
- `ops.add(a, b)` (when used in a type and with `^`) means that the type has to implement the interface ops.add 'adding' types a and b.
- `n(int)` just means cast `int` to `n`
  - `n(int) ? n(int) : int` just means if you can cast `int` to `n` then it is type `n(int)` else it is `int`
  - `a ?? b` just means `a ? a : b` so in this case it is identical to above
- `lte` just means less than or equal.

So we could create a type to use in fib like this;

```rust
// note: just MVP you may want more methods or whatevs
// represents something in the form of coeff * 10 ^ exp.
factor = (coeff: int, exp: int) :: {
    @implement(ops.sub, fn (a: factor, b: int) => {
        new_data = a.coeff * 10 ** a.exp - b;
        new_exp = int(math.log10(new_data));
        return factor(new_data / 10 ** exp, new_exp);
    })

    @implement(ops.lte, fn (a: factor, b: int) bool => {
        return a.coeff * 10 ** a.exp <= b;
    })
};
```

> Slight note: a struct/type is just a tuple bounded to some methods/static members so in this case `factor(a, b)` is really just `factor((a, b))` (that is cast the tuple `(a, b)` to `factor`) however you can leave out the extraneous tuple parentheses in this case.

You could of course overload ops.sub (you can only overload operators in Porc) with ones for subtracting factors for one another and other operations.

Now if we said `fib(factor(2, 1))` we would get `17711`!

## Generic Methods/Type Structures

Generic methods are just simply not needed since in essence they are generic already!  However often we want generic type structures for example let's say we want a vector (that is something that is closed over addition and that snizz not the growable array from C++) and in actual fact we could use our vector in our fib example too!

Well let's start with a naive implementation;

```rust
Vec = (x, y, z) :: {
    @implement(ops.add, fn (a: vec, b: vec) => {
        return vec(a.x + b.x, a.y + b.y, a.z + b.z);
    })

    @implement(ops.sub, fn (a: vec, b: vec) => {
        return vec(a.x - b.x, a.y - b.y, a.z - b.z);
    })

    // and so on...

    @implement(ops.add, fn (a: vec, b: int) => {
        // presuming the `int` as a one dimensional vector
        return a + vec(b, 0, 0);
    })

    // and so on...
};
```

> We will come back to a better implementation in a minute just first another question!

Now that is a lot of typing, also what are the types of x, y, and z??  If they are the crazy weird `any ^ ...` stuff above then if we wanted to print it how would we?  Or how would you access the `x` member as an int?  And so on.

A better example to illustrate this would be if we had a priority queue and wanted to grab an element, what would be the type of that element?  You couldn't put an int into the structure and get an int out.  Well there are two ways of accomplishing this...

### First way - call site based

Let's say that the method `queue.dequeue()` returns some object and in this case we have put a sequence of integers into the queue how would we get them out?

```rust
while (!queue.is_empty()) {
    obj = queue.dequeue;
    // I can't just do `print(obj);`
    // I can either do a cast (unsafe cast)
    print(unsafe(int(obj))); // raises runtime error if fails
    // or I can check the type
    // `is` effectively checks if the obj can be an int not necessarily if it is
    // use `==` to check if it is explicitly an int
    if (obj is int) {
        print(int(obj));
    } else {
        // do whatever error checking you want
    }
}
```

A few things to note:

- Implements and variants (we will get to these soon) are compile time only
  - So you **can't** say `if (obj is any ^ ops.add(obj, obj))` or similar
  - You can only compare concrete types i.e. `if (obj is vec)` or `if (obj is int)`.
- Type checking is pretty free (each object carries a typetag with it so it is just an integer comparison)
- Unsafe blocks are useful in some cases to reduce verbosity but aren't recommended
  - `--Wno-unsafe` flag turns on errors for any unsafe blocks.
  - Unsafe can either be used like a function (like this) or a block i.e. `unsafe { /* ... */ }`

### Second way - embed type into the structure

Since type tags exist we can embed a typetag into a structure and thus use that as the type for all the contents.

This can be used like; `Queue(int)` for example it would look something like this;

```rust
// Just a MVP implementation
// note: giving default value to contents so that you could pass your own array if you want
Queue = (T: Type, contents: [...]T = []) :: {
    // note: we have the void return here so we don't get any weird
    //       returns from the append typically the style is that if a function
    //       is void you add the void return type!
    enqueue = (self, val: T) void => self.contents.append(val);
    dequeue = (self) => self.contents.pop();
};

// Using it like
queue = Queue(int);
queue.enqueue(4);
queue.enqueue(9);
// you could also write it as (more of a side-effect of contents being in the tuple)
queue = Queue(int, [4, 9]);
print(queue.dequeue()); // 4
print(queue.dequeue()); // 9
```

## Back To Vec

Now we are going to take this and apply it to our vector and also apply some better ways to write out the vector;

```rust
// @auto_type() can be used in the case where you don't want the api user to have
// to type out the type each time if you can get it from the variables
// Note: You could also just say `T: Type` but then require user's to give
// the type every time.
Vec = (x: T, y: T, z: T, @auto_type(T)) :: {
    // element_wise just applies the operation on each of the given values
    // i.e. Vec(a.x + b.x, a.y + b.y, a.z + b.z);
    @implements.element_wise(ops.add, x, y, z);
    @implements.element_wise(ops.sub, x, y, z);
    // so on

    // note: we are stating that the comparison is purely between the first member
    @implements.func(ops.lte, (a: Vec, b: int) => {
        return a.x <= b;
    });

    // methods/variables starting with `_` are private
    // in this case we are stating that an integer value is 1D vector
    _from_int = @conv_from(int, (a) => vec(a, 0, 0));

    // .cast just casts the second type to the 'struct type' i.e. Vec(int)
    // then just applies the operator.
    @implements.cast(ops.add, int);
    @implements.cast(ops.sub, int);
    @implements.cast(ops.lte, int);
    // and so on...
};

// it can be deducted
flt_vec = Vec(2.5, 3.9, 2.1);
// if we want these to be floats we have to specify else it'll be ints
needs_type = Vec(2, 3, 4, flt);
fib(Vec(1, 2, 3)); // 1
fib(Vec(4, 2, 0)); // 5
```

> You could also apply `@auto_type` to Queue for the cases where you pass an array.

Now I implemented some of the functions just so it would work for fib in reality a lot of these are malformed so take it as what it was meant to be an example to show that our structs are kinda duck typed (but statically so).

## Side Note: why no `<T>` generics

Syntatically having something like `queue = Queue<int>()` would be perfectly fine however `<T>` is insanely complicated to parse especially with our complicated types for example `a < b ? 1 : 2 > c` is perfectly valid Porc and adding generics here would require some lexical analysis to detect exactly what you are trying to do.

Also type tags in functions really should be avoided are maybe will eventually be removed so the generic syntax would be for structs only which already do a good job of having the type tag like is `queue = Queue<int>()` really much nicer than `queue = Queue(int)`?

