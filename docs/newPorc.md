# Porc Syntax

Porc has gone through a series of changes (rightfully so) in terms of its type system; this has cascaded to some other aspects of its syntax.

```perl
# Comments are now like this and block comments;
/* The file scope is no longer 'completely' separate to a function scope
 * It merely just allows a few different ways of writing things
 * you also have to use `mut` to define things
 * by default all global variables are constant (well rather they can't be edited in a local sense
 */
mut x = 1 + 2;
# and to define a constant it is just
const y = 2;
println(x); # 3
# for multiple definitions it is just;
mut a, b = 0, 1;
# and to apply const
const c, d = 5; # both c and d are const and 5

# Macros can't operate on unique syntax so easily now
# Enums replace most things or for example in this case the following;
cmd = @import("cmd");

# if you wanted to import all into current scope
@import("cmd");

# you can specify types (default is expr)
macro @ourEval(expr: AST.Expr) {
  # it does cache function compilations
  evaluated = @compiler().Compile(expr, .FastCompile);
  result = @interpreter().evaluate(compiled);
  <[ result ]>
}

# other file scopes
# Note: switch -> match to more closely match its use
fn fib(n) {
  match n {
    # | to designate a case (previously you could not use this)
    # the reason why is to not only make it cleaner
    # but to also prevent ambiguities when returning match
    | 0, 1 => 1
    | _    => fib(n - 1) + fib(n - 2);
  }
}

println(fib(3));      # calculates fib at runtime
println(@ourEval(fib(3))); # calculated at compile time

# sometimes you want macros to generate code you can use the following;
macro @printRange(range: AST.PostfixExpr.Range) {
  <[
    for (i in range) {
      print("${i}, ");
    }
    println();
  ]>
  # it is smart enough to realise when you want to insert AST
  # and when you want to keep it as an abstract object
}

# this hasn't changed but recap
printRange(0..5);  # 0, 1, 2, 3, 4
printRange(0..=5); # 0, 1, 2, 3, 4, 5
# and step;
printRange(0..10:2); # 0, 2, 4, 6, 8
# backwards
printRange(9..=0); # 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
# you can represent infinity as;
mut infRange = 0..;
println(infRange[0]); # 0
println(infRange[1]); # 1
mut it = infRange.it();
println(it.next()); # 0
println(it.next()); # 1
println(it.next()); # 2

# and going negative...
mut negRange = 0..:-1;
# note: you can use ranges in arrays / collections for splicing
println(negRange[..10]); # 0, -1, -2, -3, -4, -5, -6, -7, -8, -9
# ^^ Note: you can leave out the LHS it defaults to 0

# technically you can also specify what is called the 'universal range'
# this is not printable but can be used to represent the entirety of a set
# very rarely is this important
mut universalRange = ..;
mut exampleArray = [1, 2, 3];
println(exampleArray); # [1, 2, 3]
println(exampleArray[..]); # [1, 2, 3]

# type system is more nuanced
fn add1(a, b) {
  return a + b;
}

# and now you can just use generics
# it makes the type system much more clean
fn add2(a: $A, b: $B) {
  return a + b;
}

# integer division is now // instead of %/
intDiv = 3 // 2; # 1 instead of 1.5

println(add1(1, 2)); # 3
println(add2[int, int](1, 2)); # 3
# buttttt you can also not include the types
# it'll figure it out itself.  It is smart!

println(add2("wow", " cool")); # wow cool
println(add2[flt, int](5.4, 2)); # 6.4 (approx)

@handleError(.TypeConstraints, .SkipExpr | .PrintError) <| {
  println(add2(2, "wrong")); # COMPILER ERROR
  println(add1(2, "wrong")); # COMPILER ERROR
}

# generics can even be value based
# note: you specify the '$' around the name not the type
const factorial = ($n: int) where n >= 0 => {
  if (n <= 1) 1
  else        factorial[n - 1]() * n;
}

# now we can just designate a type of a function
const make_partial = ($func, $args, actual_args) => {
  return @invoke(func, args, actual_args);
}

const add = (a, b) => a + b;
const add5_normal = (a) => add(5, a);
const add5_using_generics = make_partial[add, 5];
assert(add5_normal(10) == add5_using_generics(10) == add(5, 10));

# and you can specify like;
const call_f = (f : fn[$T](List[T])->void, data: List[T]) => {
  f(data);
}

call_f[flt](println, [1.0, 2.0, 3.0]);

mut map = ["2": 2, "3": 3, "bob": 9];

mut curr  = ["2": 2, "3": 3, "bob": 9];
mut init  = Map{ { "key", 1 }, { "wow", 2 }, ["Coolies"] = 9 };
mut macro = @NewMap { "2": 2, ["cool"] = 49 };
mut best  = @NewMap(("cool", 2), ("nice", 9));
mut other = zip() <| [("cool", 2), ("nice", 4)];

# and for collections the generics have impacted them too!
# NOTE: you have to specify size for array
# (of course it'll type deduce it for you)
mut array: Array[int, 3] = [1, 2, 3];
# use the 'universal range' to denote that you don't want
# an array to be initialised (slightly odd perhaps in terms of syntax)
mut uninit: Array[flt, 100] = ..;

# you can also specify argument types as constant as usual
# (just designates you can't edit that tuple value)
fn func(const x: int) {
  @handleError(.ConstEdit, .SkipExpr | .PrintError) <| {
    x = 2;
  }
}

# lists are identical
mut list: List[int] = [1, 2, 3];
```
