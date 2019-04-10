# Type System

We want to be able to say things like;

```rust
fn add(a: int | flt, b: a) -> a {
  if (a == int) return a + b;
  else if (a == flt) return a - b;
}

Array = @DefMacro((T: AST.TypeExpr) => {
  
});

strings : @Array(int) = @Array(int).new();
```
