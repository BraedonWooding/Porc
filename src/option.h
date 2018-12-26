/* 
    MIT License

    Copyright (c) 2018 Braedon Wooding

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef OPTION_H
#define OPTION_H

// for _Static_assert
#ifndef _Static_assert
#include <assert.h>
#endif

#include <stdbool.h>

#define OPTION_TYPE void*
#define OPTION_SIZE sizeof(OPTION_TYPE)

#define Option(x)   _Static_assert(sizeof(x) <= OPTION_SIZE, \
                        "Warning object size would cause overflow into option internals: " #x); \
                    Option

typedef struct _option_t {
    bool has_value;
    OPTION_TYPE obj;
} Option;

#define IS_SOME(opt) ((opt).has_value)
#define IS_NONE(opt) (!(opt).has_value)

#define SOME(x) ({ \
    Option __option__ = {.has_value = true}; \
    _Static_assert(sizeof(x) <= OPTION_SIZE, \
        "Warning object size would cause overflow into option internals: " #x); \
    typeof(x) *tmp = (typeof(x)*)(&__option__.obj); \
    *tmp = x; \
    __option__; \
})

#define NONE ((Option){.has_value = false})

#define OPT_FORCE(opt, type) ({ \
    if (!opt.has_value) { \
        fprintf(stderr, __FILE__ ":%d: UNWRAP FAILED FOR: " #opt, __LINE__); \
        abort(); \
    } \
    *(type*)opt.obj; \
})

#define OPT_IF_LET(opt, type, block) ({ \
    if (opt.has_value) { \
        type out = *(type*)opt.obj; \
        block \
    } \
})

#endif