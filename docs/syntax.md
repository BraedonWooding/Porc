# Syntax on a page

Well not necessarily a page but just the simplest way to present the syntax.

```rust
// lines end with `;`
// all global variables have to be constant
const variable = 5;
// you can declare just the type
const hello_world: str;

// if/for/while/.../etc are all expressions
// enums can be inferred like this (`.Windows` becomes `std.os.Kind.Windows`)
// enums have upper camel case (so do types except for the primatives)
hello_world = if (std.os.kind == .Windows) "Hello Windows" else "Hello Posix";

// you can declare functions like this
// types are inferred, functions also have lower snake case
// countries is an array of tuples (str and int) that is named
fn printOutput(hello, age, countries: (country: str, years: int)[]) {
  name = std.io.input("Enter Name: ");

  // `<-` grabs whatever is to the right and puts it inside the function
  // i.e. 1 + a(2) <- b + 1 == 1 + a(2, b) + 1
  // `->` does the other way i.e. 1 + a -> b(2) + 1 == 1 + b(a, 2) + 1
  countries = std.lazy.apply(countries) <| (c) => {
    suffix = if (c.years == 1) "year" else "years";
    // expression based statements means last statement returns
    // if not ending with `;`
    "{c.country} ({c.years} {suffix})"
  };

  println("{hello}, my name is {age}, I live in the following countries; {countries}");
}

// you can also declare functions like this
// return type of main can be a combination of `int`, `void`, and `error`
// note: return types are inferred but in this case it is `error | void`
main = () => {
  // `|` means or in this case so it can either be `int` or `void`
  age: int | void = int.parse(std.io.input("Enter age: "));
  // `void` can be checked as a boolean
  // (or rather you can check if age holds a value that isn't void)
  if (!age) return error.new("Enter a valid age");
  // The type is automatically inferred as being just `int` from now on
  // `...` meaning it can grow

  countries: (str, int)[...];
  // `let` is the way to do an assignment that returns a result
  // ctrl+d just returns an empty string so we can check it as a boolean
  while let country = std.io.input("Enter country (Ctrl+D to stop): ") {
    years = int.parse(std.io.input("Enter years spent in {country}"));
    if (!years) error.print("Invalid year");
    else        countries.append((country, years));
  }

  print_output(hello_world, age, countries)
  // for loops work like you would expect
  for fruit in ["Apples", "Oranges", "Pears"] {
    println("Fruit {fruit}");
  }
  // or (use 0..=10 for inclusive) for range based
  for i in 0..10 { }
  // we don't have traditional for loops
  // you can have multiple like this

  for x, y in [(1, 2), (3, 4), (5, 6)] {}

  // i.e.
  for key, value in ["America": 2, "Australia": 4, "Canada": 9] {}
};
```

## Extra Tidbits

Just incase you are interested, these are some nicer ways to write things in some cases.

```rust
// typically recommended to state return types for readability
main = () error | void => {
  // you can simplify error handling like
  // `x ?? y` is just the same as writing { z = x; if (z) z else y }
  age: int = int.parse(std.io.input("Enter age: "))
             ?? return error.new("Enter a valid age"); // valid return
  // or can just do a force cast (panics on failure)
  age: int = @ForceCast(int, int.parse(std.io.input("Enter age: ")));

  // you can do list comprehension like this;
  countries = for c in countries {
    suffix = if (c.years == 1) "year" else "years";
    "{c.country} ({c.years} {suffix})"
  }
  // the lazy approach just is often faster as you aren't creating a lot of extra
  // GC'd junk
};
```
