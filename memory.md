# Memory Management in Porc

Memory management in Porc is meant to be as simple as possible and typically you won't ever have to care about memory.  However just incase you want to understand what happens here it is;

## Static Memory

Porc has a step where it calculates the static memory for things for example in the following;

```C
main = () => {
    x = 3;
    y = 9;
    z = x + y;
    q = z * 10;
    println("Z: {z} Y: {y}, X: {x}, Q: {q}");
    return str(z);
};
```

That'll produce some IR code like;

```assembly
func_new $main 0 1 %str /* Returns string and takes nothing */
func_reserve_mem $main 4 %int %int %int %int
/* A ton of other code */
func_label $main $main.enter /* begin main block */
new_var $x %int 3
new_var $y %int 9
new_var $z %int
arith_add %int 2 $x $y $z
new_var $q %int
arith_mul %int 2 $z 10 $q
new_str $temp1 21
str_format $temp1 "Z: {} Y: {}, X: {}, Q: {}" z y x q
new_str $main.ret.0 0 /* no initial size just for the sake of showing it */
conv %int %str $z $main.ret.0
track_var $main.ret.0 /* Garbage collect it */
free_str $temp1 /* Local scoped so no need to release to garbage collector */
ret_val 1 $main.ret.0
func_label $main $main.exits
```

Now as you can see a lot of the code doesn't have to be garbage collected
