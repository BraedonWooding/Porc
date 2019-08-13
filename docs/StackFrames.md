# Stack Frames in Porc

Stack frames in Porc behave very differently then in other languages.  This mainly gives Porc the benefit from having thier stack frames grow into the heap more easily and effectively infinitely.  Helps with preventing recursion death.

## Slight note on recursion

Of course with recursion eventually you will run out of space even with heap allocated stack frames...  However it can be useful in some cases for example it means we can be more conservative on the size of our default stack and not cause problems for people.  Also means we can allocate more objects on the stack freely preventing heap allocations and worst case just cause a psuedo buffer allocation.  For example most strings are allocated on the stack unless they are returned and even then we can optimise them such that in some cases it can be allocated in the previous frame.  Another example would be tuples which can be fully allocated on the stack (most of our tuples aren't boxed).

We also perform heavy tail call recursion optimisation and even change callsites/functions to make them recurse better.

Finally recursion actually has a default limit of 1024 iterations (currently arbitary) and you can either remove this limit or increase/decrease it on each function independently and technically can even do it independent on each call!

This is done through context variables, though keep in mind that due to how context works you can't change context of 'x' function inside 'x' function, so you couldn't change the recursion limits inside the recursive function.

## How are they done

Basically we have a stack like object of stack frames that contains a few objects inside each 'frame'.  The first one is a pointer to the allocated memory.  Typically this will point to the stack.  It also stores the stack size (capacity) and the current size.  Both of these are stored as 2 byte integers which does impose a ~64kb limit (technically actually 63.999 kb) on individual frames (not relevant for arrays and large tuples will be boxed so won't apply to them either).  This is just because in all honestly even this is much larger than we probably even want but 256 bytes is a lil too small.  The perfect amount would be around 8kb honestly.

We also store the current top of the stack ptr close to the actual stack memory.  This is just to improve prefetch hints.  Basically the less that we can jump around in memory the better.  So often we prefer to unbox tuples and to keep pts close to their data.

## Can Stack frames grow

Yes stack frames can grow/shrink as explained above with capacity and current size.  They can't allocate more memory though.

## How does this impact GC

We just traverse all stack frames for all threads checking all objects up till and stopping at the current size.
