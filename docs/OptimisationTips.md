# Optimisation in Porc

## Choosing correct options

If you compile and run porc like `porc debug x.porc` then of course you will expect it to be slower as it'll not perform destructive optimisations (equivalent to making all objects volatile in C).

As a baseline the recommended is instead of `porc run x.porc` use `porc compile x.porc -o a.cporc && cporc a.cporc` the reason why is that `compile` will perform more heavy optimisations since it isn't worried about compiling fast where as `run` just performs some baseline optimisations (mostly register focused) that are very quick to perform.

> Note: cporc is the standard interpreter you may decide to use a different one such as porc-tiny (meant for embedded).  Also note that you can provide the options at either the compile or when calling cporc (or both, cporc options override the porc ones!).

To optimise further you can customise various aspects of the system;

## Memory Allocation

- Increase stack space (useful since otherwise stack frames are allocated on a stack and this is not only slow but can impact GC speed)
  - `--stack-size=<bytes>` i.e. `--stack-size=4096` (4kb)
  - You can prefix it with `mb` or `kb` to designate it that way as well.
  - If we can't allocate this section of space then the program will crash upon initilisation.
- Use preallocated (growable) heap rather than bucketed heap.  This can be more efficient if mark and sweep and compact algorithm is used.
  - `--init-heap-size=<bytes>` i.e. `--init-heap-size=4mb` (4mb)
  - Same as before you can prefix, no prefix is bytes
  - Recommended to also enable `--gc-compact`
  - This will result in a higher throughput GC (less time spent in GC) however each individual GC cycle will take longer (higher latency)

### Bucketed Heap

The standard algorithm for memory allocation is something called a bucketed heap.  Basically we allocate memory in something called buckets where each bucket represents a different type of memory allocation.

The first one is for small objects (size <= 32 bytes) and they are allocated using a bumping pointer like method.  A linked list of fixed size allocation blocks are used.

For each allocation block there are two possible algorithms;

- Compaction
  - Higher latency but higher throughput, allocations are O(1) (unless GC triggers so amortized)
- Linear Allocation
  - Lower latency but lower throughput, allocations are O(k) where k is the size of the fixed buffer (often 256).

The compaction works by moving all memory to new buffers when it is declared alive and then the old position holds the value of the new position a temporary bitarray is created to denote which objects have been moved over and then this means that we don't pay the cost of having to rego through the stack at the end.

On the otherhand linear allocation always has the bitarray active and it represents which 'slots' are filled so each allocation just scans this for an empty slot.  It has a clever trick where it checks if the middle element is filled in which it goes from the back to the front.  This generally results in an amortized result of `O(k/2)` (relevant since k is fixed and we are talking about preference for low latency allocations) however this of course degrades over time as the slots become arbitarily open/closed.

You can choose it via 3 ways; `--gc-small=linear` or `--gc-small=default` (default is linear allocation), `--gc-small=compact` (compaction), `--gc-small=periodic_compact` (will perform a compaction run periodically to prevent degradation of slots).

The second bucket is for objects of size > 32 bytes but <= 100 bytes.  This is allocated using a similar 

### Preallocated (growable) heap

This by default uses

## Writing Code to Fit Registers


