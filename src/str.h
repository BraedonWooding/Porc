#ifndef BWW_STR_LIB
#define BWW_STR_LIB

#include <stddef.h>

#ifdef __GNUC__
    #define _attr_(A) __attribute__((A))
#else
    #define _attr_(A)
#endif

// Warns on unused return value.
#define _warn_unused_ _attr_(warn_unused_result)

// Gurantees no aliasing for newly allocated data.
#define _malloc_ _attr_(malloc)

typedef char *Str;
typedef char *StrIt;
typedef void *StrFmt;

_warn_unused_ _malloc_ Str str_empty(void);
_warn_unused_ _malloc_ Str str_new(char *text);
_warn_unused_ _malloc_ Str strn_new(Str text);
_warn_unused_ size_t str_len(Str str);
void str_free(Str a);
_warn_unused_ _malloc_ Str str_new_prepare(size_t size);
void str_prepare(Str *str, size_t size);
_warn_unused_ _malloc_ Str str_cat(char *a, char *b);
_warn_unused_ _malloc_ Str strn_cat(Str a, Str b);
void str_append(Str *a, char *b);
void strn_append(Str *a, Str b);
void str_prepend(Str *a, char *b);
void strn_prepend(Str *a, Str b);
_warn_unused_ _malloc_ StrIt str_split(Str a, char *b);
_warn_unused_ _malloc_ StrIt strn_split(Str a, Str b);
void str_finish_it(StrIt it);
void strit_move_next(StrIt *a);
void str_extend(Str *str);
void str_append_char(Str *a, char c);
char *str_extract(Str a);

#endif
