# Porc Scratch Pad

Porc takes a lot from functional languages i.e.

```c
sqr :: (x) => x * x;

main :: () => {
  data := [1, 2, 3, 4, 5];
  // imperative way
  for (a in data) println(sqr(data));
  // more functional way/porc way
  @foreach(data) <| @partial(x, println <| ((x) => x * x)(x));
  // or written out explicitly
  @foreach(data) <| (x) => println <| ((x) => x * x)(x);

  // however the fact we have to call the function in that semi ugly way
  // is a tad annoying so that is what the compose syntatic sugar is for
  // also we can replace the foreach with a standard stream operator
  data >> println <- (x) => x * x;

  // you can also invert these freely but you do have to use pretty heavy
  // parentheses as its much less clear to the compiler what you mean
  (((x) => x * x) -> println) >> data;

  // you can also partially evaluate things using it i.e.
  // note: you have to designate an inner tuple like this
  data >> println <- ((x, y) => x + y).tcall((y = 10));
  // is identical to
  data >> println <- (x) => x + 10;
  // you can also write it out long hand
  @foreach(data) <| (x) => println <| ((x, y) => x * y)(x, 10);
}
```

Example on the unclearness without parentheses

```c
(x) => x * x -> println
```

Is it...

- `((x) => x * (x -> println))`
- `((x) => ((x * x) -> println))`
- `((x) => x * x) -> println`

This ambiguiity is a key problem, the real ambiguity is between case 2 and 3, the first is more just a precedence issue.  In a slightly bigger example;

```c
foo :: (func: (a: int) -> (()->())) => func(2) -> println;
k :: foo((x) => {
  println(x);
  return () => println("Ran foo");
});
```

- This basically could have 2 return types for foo
  - The first being nothing since its composed from println
    - i.e. it is `foo :: (x) => println(((func) => (func(2)))(x));` or in partial macro format; `@partial(x, println <| ((func) => func(2))(x))`
  - The other being it expecting to return a function that prints out the result of `func(2)(...)` i.e. `foo :: (func) => (x) => println(func(2)(x));`

Or in other terms is the println wrapping foo or is it wrapping func(2).  This ambuigity means that you have to designate parentheses in these cases.

## Goals of Porc

- Proper functional programming support
- Dynamic Language

## Types in Porc

```c
// vector is an interface with no members
type Vec is interface {
  add :: (this: Vec, other: Vec) => {
    // while we can do;
    // return @zipWith(op.add, tuple(this), tuple(other));
    // we instead should try to make it a bit more robust by instead having a method
  }

  // no value so inherited classes have to implement
  // we are specifying that the type has to implement the Num interface
  // it is smart enough to know that the type Vec will change
  // and is happy to update it correspondly.
  iter : (this: Vec)->Iter[Num];
}

// a post block is a block after the macro
// it is extracted from the outside scope into this macro if it exists
// you can force an error to be raised by removing the `void`
macro VecND(n: AST.Constant.Int, block: AST.PostBlock | void) {
  name :: "Vec" + n + "D";
  args_value := AST.TupleValueDecl();
  T := AST.Type.Generic("T"):
  for (i in 0..n) args.add((name: "_" + i, type: T, default: 0));
  // this is a convenience function to convert from a value decl to type
  args_type := AST.TupleTypeDecl();

  // use '\' to refer to outside elements
  // you can have types subclass primitives like Array
  // note: we don't have to specify its a Vec that is implicit
  //       but we do just to get nice error checking that we implemented it
  //       right
  // normally it's standard to specify all types fully in macros
  // just to prevent possible errors.
  // NOTE: you can even using comments for any docgen
  <[
    type \name[T: Num] is Vec, Array[T] {
      /*
        Construct a new \name from given values.
        Default is 0 for each coordinate.
      */
      new : \args_type : \args_value -> \name => {
        return \name(Array[T]());
      }

      iter : (\name)->Iter[Num] : (this: \name)->Iter[Num] => {
        return Array[T](this).iter();
      }
    }
  ]>

  if (block) {
    <[
      type \name[T: Num] \block
    ]>
  }
}

@VecND(3) {
  // you can use maps to create properties
  @property(x, T, this[0]);
  @property(y, T, this[1]);
  @property(z, T, this[2]);

  // you can define more functions
  cross_product :: (this: Vec) => {
    // ...
  }
}

@VecND(2) {
  // you can create a property by setting up a map
  @property(x, T, this[0]);
  @property(y, T, this[1]);
}

main :: () => {
  a := Vec2D(1, 2);
}
```
