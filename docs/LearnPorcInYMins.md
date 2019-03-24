# Learn Porc In Y Mins

This is meant to be a very quick guide to learn Porc.  You should be able to pick it up really quickly if you have knowledge in C like languages (C/C++/C#/Java/Python/Rust/Julia/etc).  I'm not going to presume any experience though.

```rust
// Line Comments are C styled
/* Block Comments are C styled /* and can be nested */ */

//                         //
/* 1. Primatives/Operators */
//                         //

// are 'statements' end with `;`
x = 3;      // this is an integer
x = 5.5;    // a floating point number
x = true;   // a boolean (true/false)
x = "hey";  // a string

// math works!
y = 1 + 1;  // => 2
y = 4 - 1;  // => 3
y = 94 * 2; // => 188
y = 90 / 9; // => 10
y = 9 / 2;  // => 4.5

// integer division
y = 9 %/ 2; // => 4

// modulo
y = 4 % 3;  // => 1

// powers (x ** y => x to the yth power)
y = 4 ** 2; // => 16

// Comparison operators
z = 2 == 2; // true
z = 4 != 2; // true
z = 4 > 3;  // true
z = 4 < 3;  // false
z = 4 <= 3; // false
z = 4 >= 4; // true

// use `&&` and `||` to designate and / or (short circuits)
z = true && false; // false
z = true || false; // true

// for a < b < c you would normally write
z = 2 < 3 && 3 < 5;

// you can just write using chaining
z = 2 < 3 < 5;

// you can group primatives in a tuple
t = (1, 2, 3);
println(t[0]); // => 1
t = (x=1, y=2, z=3);
println(t[0]); // => 1
println(t.x);  // => x

//              //
/* 2. Functions */
//              //

// define it
fn fib(n) {
  if (n <= 1) return 1;
  return fib(n - 1) + fib(n - 2);
};
// ^ this way they can't be redefined and this is more suited for structures/top level defs

// lambda like
fib2 = (n) => n <= 1 ? 1 : fib2(n - 1) + fib2(n - 2);

// technically a function is just a tuple that is bounded to a block, which returns a tuple

// call them like you would
println(fib(4)); // 5

//            //
/* 3. Structs */
//            //

// like functions structs consist of a tuple bounded, however it is bounded to multiple members rather than just one.
Vec2D = (x, y) :: {
  fn modulus() {
    return math.sqrt(x * x + y * y);
  };

  fn angle() {
    return math.atan(y / x);
  };
};
```
