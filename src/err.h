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

#ifndef BWW_ERR_H
#define BWW_ERR_H

#include <assert.h>

/*
    Even though compilers can make this smaller we want multiple error types
    to work together reasonably well so we need this!
*/
#define ERR_TYPE int
#define OK_TYPE unsigned long long int
#define ERR_SIZE sizeof(ERR_TYPE)
#define OK_SIZE sizeof(OK_TYPE)
#define RESULT_CONTAINER_SIZE (ERR_SIZE > OK_SIZE ? ERR_SIZE : OK_SIZE)

/*
    Comment out this out if you want each error to not get a unique ID
    This will mean that errors are suitable together.
*/
#define UNIQUE_ERRORS

typedef enum _result_variant_t {
    ERROR,
    VALUE,
} result_variant;

typedef struct _result_t {
    union {
        OK_TYPE ok;
        ERR_TYPE error;
    };
    result_variant variant;
} Result;

#define Result(type) Result

#ifndef STRINGIFY
#define STRINGIFY(x) case x: return #x;
#endif

#ifdef UNIQUE_ERRORS
#define ENUM_CASE(x) x = __COUNTER__,
#else
#define ENUM_CASE(x) x,
#endif

#ifndef CONCATENATE
#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2)  arg1##arg2
#endif

#ifndef FOR_EACH
#define FOR_EACH_1(what, x)         \
    what(x)

#define FOR_EACH_2(what, x, ...)    \
    what(x)                         \
    FOR_EACH_1(what, __VA_ARGS__)

#define FOR_EACH_3(what, x, ...)    \
    what(x)                         \
    FOR_EACH_2(what, __VA_ARGS__)

#define FOR_EACH_4(what, x, ...)    \
    what(x)                         \
    FOR_EACH_3(what,  __VA_ARGS__)

#define FOR_EACH_5(what, x, ...)    \
    what(x)                         \
    FOR_EACH_4(what,  __VA_ARGS__)

#define FOR_EACH_6(what, x, ...)    \
    what(x)                         \
    FOR_EACH_5(what,  __VA_ARGS__)

#define FOR_EACH_7(what, x, ...)    \
    what(x)                         \
    FOR_EACH_6(what,  __VA_ARGS__)

#define FOR_EACH_8(what, x, ...)    \
    what(x)                         \
    FOR_EACH_7(what,  __VA_ARGS__)

#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__) 
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N 
#define FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0

#define FOR_EACH_(N, what, ...) CONCATENATE(FOR_EACH_, N)(what, __VA_ARGS__)
#define FOR_EACH(what, ...) FOR_EACH_(FOR_EACH_NARG(__VA_ARGS__), what, __VA_ARGS__)
#endif

#define ERR_ENUM_GET_NAME(name, ...)                \
    char * name##_to_string(name val) {             \
        switch(val) {                               \
            FOR_EACH(STRINGIFY, __VA_ARGS__)        \
            default: return ((void*)0);             \
        }                                           \
    }

#define GENERIC_ERR_HANDLER(val)                        \
    else if (err > val##_begins && err < val##_ends) {  \
        return val##_to_string(err);                    \
    }

#define ERR_ENUM_GROUP_H(name) \
    char *name(int err);

#define ERR_ENUM_GROUP(name, ...)                       \
    static inline char * name (int err) {               \
        if (0) { }                                      \
        FOR_EACH(GENERIC_ERR_HANDLER, __VA_ARGS__)      \
        else {                                          \
            return ((void*)0);                          \
        }                                               \
    }

#define ERR_ENUM_C(name, ...) \

#define ERR_ENUM(name, ...) \
    typedef enum _##name##_t { ENUM_CASE(name##_begins) FOR_EACH(ENUM_CASE, __VA_ARGS__) ENUM_CASE(name##_ends) } name; \
    static inline ERR_ENUM_GET_NAME(name,__VA_ARGS__)

#define ERR(err) (Result){ .error = err, .variant = ERROR }

#define OK(x) ({ \
    Result __result__ = {.variant = VALUE}; \
    _Static_assert(sizeof(x) <= RESULT_CONTAINER_SIZE, "Warning object size would cause overflow into result internals: " #x); \
    typeof(x) *tmp = (typeof(x)*)(&__result__.ok); \
    *tmp = x; \
    __result__; \
})

#define IS_ERR(x) (x.variant == ERROR)

#define IS_OK(x) (x.variant == VALUE)

#define UNWRAP(x, type) ({ \
    _Static_assert(sizeof(type) <= RESULT_CONTAINER_SIZE, "Warning object size would cause overflow into result internals: " #type); \
    Result __result = x; \
    *((type*)&__result.ok); \
})

#define IF_LET(x, type, block) \
    { type out = UNWRAP(x, type); if (IS_OK(x)) block }

#define TRY(res, type) ({                               \
    Result result = res;                                \
    if (IS_ERR(result)) return ERR(result.error);       \
    UNWRAP(result, type);                               \
})

#define EXPAND_EXPR(x) x;

#define GUARD(cond, err, ...) if (!(cond)) { return ERR(err); FOR_EACH(EXPAND_EXPR, __VA_ARGS__) }

#define WRAP_MAIN(name, handler)            \
    Result name(int argc, char *argv[]);    \
    int main(int argc, char *argv[]) {      \
        Result res = name(argc, argv);      \
        if (IS_ERR(res)) {                  \
            fprintf(stderr, "ERR (%d): %s\n", res.error, handler(res.error)); \
            return 1;                       \
        }                                   \
        return UNWRAP(res, int);            \
    }                                       \
    Result name(int argc, char *argv[])

#define WRAP_MAIN_ENVP(name, handler)       \
    Result name(int argc, char *argv[], char *envp[]);    \
    int main(int argc, char *argv[], char *envp[]) {      \
        Result res = name(argc, argv, envp);      \
        if (IS_ERR(res)) {                  \
            fprintf(stderr, "ERR: %s\n", handler(res.error)); \
            return 1;                       \
        }                                   \
        return UNWRAP(res, int);            \
    }                                       \
    Result name(int argc, char *argv[])

#endif