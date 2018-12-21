#include "str.h"
#include "str_prefs.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#define ALPHABET_LEN 256

#define max(a, b) (((a) < (b)) ? (b) : (a))

// predeclare for function
struct _str_it_t;

typedef void(*fn_move_next_it)(StrIt *it);
typedef void(*fn_free_data)(StrIt it);

// Handle_OOM returning either NULL or nothing
#if CRASH_ON_OOM
    #define HANDLE_OOM(mem) if (mem == NULL) abort();
    #define HANDLE_OOM_VOID(mem) if (mem == NULL) abort();
#else
    #define HANDLE_OOM(mem) if (mem == NULL) return NULL;
    #define HANDLE_OOM_VOID(mem) return;
#endif

// update when you update the struct
// only represents up to 'starting_char'
#define SIZEOF_STR (sizeof(size_t) * 2)

typedef struct _linked_list_t {
    struct _linked_list_t *next;
    void *data;
} *LL;

typedef struct _str_t {
    size_t length;
    size_t max_length;
    char starting_char;
} *Str_Impl;

typedef struct _str_ref_data_t {
    Str_Impl original;
    char *ref;
    size_t ref_len;
} *StrRefData;

typedef struct _str_find_data_t {
    struct _str_ref_data_t data;
    int delta1[ALPHABET_LEN];
    int *delta2;
} *StrFindData;

// update when you update the struct
// only represents up to 'starting_char'
#define SIZEOF_STR_IT (sizeof(void*) + sizeof(fn_move_next_it) + sizeof(fn_free_data) + sizeof(size_t) * 2)

typedef struct _str_it_t {
    union {
        StrRefData ref_data;
        StrFindData find_data;
    };
    fn_move_next_it move_next;
    fn_free_data free_data;
    size_t start_index;
    size_t end_index;
    char starting_char;
} *StrIt_Impl;

/* == Conversions ==
 * A bit esoteric in nature, to convert between them just make sure to update the sizeofs
 * We can't do these on the fly as we don't know the packing.
 */

static inline Str_Impl convert_from_str(Str str) {
    return (Str_Impl)(str - SIZEOF_STR);
}

static inline StrIt_Impl convert_from_str_it(StrIt it) {
    return (StrIt_Impl)(it - SIZEOF_STR_IT);
}

Str str_empty(void) {
    return str_new(NULL);
}

Str str_new(char *text) {
    if (text == NULL) text = "";
    size_t len = strlen(text);
    // `struct _str_t` will include the +1 for null terminating char
    Str_Impl str = s_malloc(sizeof(struct _str_t) + len);
    HANDLE_OOM(str);
    str->length = len;
    str->max_length = len;
    // include the terminating
    memcpy(&str->starting_char, text, len + 1);
    return &str->starting_char;
}

Str strn_new(Str text) {
    if (text == NULL) return str_new("");
    Str_Impl it = convert_from_str(text);
    Str_Impl str = s_malloc(sizeof(struct _str_t) + it->max_length);
    HANDLE_OOM(str);
    str->length = it->length;
    str->max_length = it->max_length;
    memcpy(&str->starting_char, text, str->length + 1);
    #if !SANTIZE_MEMORY
        if (str->max_length > str->length) {
            memset(&str->starting_char + str->length, 0, str->max_length - str->length);
        }
    #endif
    return &str->starting_char;
}

size_t str_len(Str str) {
    Str_Impl actual_str = convert_from_str(str);
    return actual_str->length;
}

void str_free(Str a) {
    Str_Impl actual_str = convert_from_str(a);
    #if SANTIZE_MEMORY
        actual_str->length = actual_str->max_length = 0;
        actual_str->starting_char = '\0';
    #endif
    s_free(actual_str);
}

Str str_new_prepare(size_t size) {
    Str_Impl str = s_malloc(sizeof(struct _str_t) + size);
    HANDLE_OOM(str);
    str->length = 0;
    str->max_length = size;
    str->starting_char = '\0';
    #if SANTIZE_MEMORY
        memset(&str->starting_char + 1, 0, sizeof(char) * size);
    #endif
    return &str->starting_char;
}

void str_prepare(Str *str, size_t size) {
    // should use realloc!!!!!
    Str_Impl actual_str = convert_from_str(*str);
    Str_Impl new_str = convert_from_str(str_new_prepare(actual_str->max_length + size));
    new_str->length = actual_str->length;
    memcpy(&new_str->starting_char, *str, actual_str->length + 1);
    new_str->max_length = actual_str->max_length + size;
    #if SANTIZE_MEMORY
        memset(&new_str->starting_char + actual_str->length + 1, 0, sizeof(char) * size);
    #endif
    str_free(*str);
    *str = &new_str->starting_char;
}

Str concatenate_strings(char *a, char *b, size_t len_a, size_t len_b) {
    Str_Impl str = convert_from_str(str_new_prepare(len_a + len_b));
    str->max_length = str->length = len_a + len_b;
    memcpy(&str->starting_char, a, len_a);
    memcpy(&str->starting_char + len_a, b, len_b + 1);
    return &str->starting_char;
}

Str str_cat(char *a, char *b) {
    return concatenate_strings(a, b, strlen(a), strlen(b));
}

Str strn_cat(Str a, Str b) {
    return concatenate_strings(a, b, str_len(a), str_len(b));
}

void append_strings(Str *a, char *b, size_t b_len) {
    Str_Impl str = convert_from_str(*a);
    size_t old_len = str->length;
    if (str->length + b_len < str->max_length) {
        // fit in string
        str->length += b_len;
    } else {
        // extend string
        str_prepare(a, str->max_length + b_len);
        str = convert_from_str(*a);
        str->length += b_len;
    }
    memcpy(&str->starting_char + old_len, b, b_len + 1);
    *a = &str->starting_char;
}

void prepend_strings(Str *a, char *b, size_t b_len) {
    Str_Impl str = convert_from_str(*a);
    size_t old_len = str->length;
    if (str->length + b_len < str->max_length) {
        // fit in string
        str->length += b_len;
    } else {
        // extend string
        str_prepare(a, str->max_length + b_len);
        str = convert_from_str(*a);
    }
    // note: these memory locations may overlap so we need memmove
    memmove(&str->starting_char + b_len, &str->starting_char, old_len + 1);
    memcpy(&str->starting_char, b, b_len);
    *a = &str->starting_char;
}

void str_append(Str *a, char *b) {
    append_strings(a, b, strlen(b));
}

void strn_append(Str *a, Str b) {
    append_strings(a, b, str_len(b));
}

void str_prepend(Str *a, char *b) {
    prepend_strings(a, b, strlen(b));
}

void strn_prepend(Str *a, Str b) {
    prepend_strings(a, b, str_len(b));
}

void ref_free_data(StrIt it) {
    StrIt_Impl actual_it = convert_from_str_it(it);
    s_free(actual_it->ref_data);
}

StrIt split_find_next(StrRefData data, StrIt_Impl old_it) {
    size_t old_end_index = old_it != NULL ? old_it->end_index : 0;
    if (old_end_index == data->original->length) {
        str_finish_it(&old_it->starting_char);
        return NULL;
    }

    size_t i;
    size_t j = 0;
    // NOTE: perfect place for some kind of string pattern matching algo
    for (i = old_end_index; i < data->original->length && j < data->ref_len; i++) {
        //printf("%c", (&data->original->starting_char)[i]);
        if ((&data->original->starting_char)[i] == data->ref[j]) {
            j++;
        } else {
            j = 0;
        }
    }
    // @CHECK: check the maths later, I'm pretty sure this is correct
    // the i < length is implicit
    if (j < data->ref_len) {
        j = 0;
        i = data->original->length;
    }
    if (i - old_end_index <= 1) {
        // there is no difference between them
        old_end_index = i;
        return split_find_next(data, old_it);
    }
    StrIt_Impl it = s_malloc(sizeof(struct _str_it_t) + i - data->ref_len - old_end_index);
    it->end_index = i;
    it->ref_data = data;
    it->start_index = old_end_index;
    it->free_data = ref_free_data;
    memcpy(&it->starting_char, &data->original->starting_char + old_end_index, i - j - old_end_index);
    (&it->starting_char)[i - old_end_index - j] = '\0';

    if (old_it != NULL) {
        it->move_next = old_it->move_next;
        s_free(old_it);
    }

    return &it->starting_char;
}

void str_split_move_next(StrIt *a) {
    StrIt_Impl it = convert_from_str_it(*a);
    *a = split_find_next(it->ref_data, it);
    if (*a == NULL) s_free(it);
    #if SANTIZE_MEMORY
        it->ref_data = NULL;
        it->starting_char = '\0';
    #endif
    s_free(it);
}

void str_finish_it(StrIt it) {
    if (it == NULL) return;
    StrIt_Impl actual_it = convert_from_str_it(it);
    actual_it->free_data(it);
    #if SANTIZE_MEMORY
        actual_it->ref_data = NULL;
        actual_it->starting_char = '\0';
    #endif
    s_free(actual_it);
}

StrIt split_strings(Str a, char *b, size_t b_len) {
    StrRefData data = s_malloc(sizeof(struct _str_ref_data_t));
    data->original = convert_from_str(a);
    data->ref = b;
    data->ref_len = b_len;
    StrIt_Impl res = convert_from_str_it(split_find_next(data, NULL));
    res->move_next = strit_move_next;
    return &res->starting_char;
}

StrIt str_split(Str a, char *b) {
    return split_strings(a, b, strlen(b));
}

StrIt strn_split(Str a, Str b) {
    return split_strings(a, b, str_len(b));
}

void strit_move_next(StrIt *a) {
    if (a == NULL || *a == NULL) return;
    StrIt_Impl it = convert_from_str_it(*a);
    it->move_next(a);
}

// If word[pos] -> word[word_len - 1] == word[0] -> word[word_len - pos]
// i.e. if the suffix of word starting from pos is a prefix of word.
bool suffix_is_prefix(char *word, int word_len, int pos) {
    for (int i = 0; i < word_len - pos; i++) {
        if (word[i] != word[pos + i]) return false;
    }
    return true;
}

void find_next(StrIt_Impl it, StrIt *a, StrFindData data);

void find_move_next(StrIt *a) {
    StrIt_Impl it = convert_from_str_it(*a);
    find_next(it, a, it->find_data);
}

void find_data_free(StrIt a) {
    StrIt_Impl it = convert_from_str_it(a);
    s_free(it->find_data->delta2);
    s_free(it->find_data);
}

void find_next(StrIt_Impl it, StrIt *a, StrFindData data) {
    if (a == NULL || *a == NULL) return;

    int start;
    if (it == NULL) {
        // first iteration
        start = data->data.ref_len - 1;
    } else {
        start = it->start_index;
        if (start == 0) {
            str_finish_it(*a);
            *a = NULL;
            return;
        }
    }

    int i = start;
    char *word = &data->data.original->starting_char;
    int j;
    while (i < start + 1) {
        j = data->data.ref_len - 1;
        while (j >= 0 && word[i] == data->data.ref[j]) {
            i--;
            j--;
        }
        if (j < 0) break;
        i += max(data->delta1[word[i]], data->delta2[j]);
    }

    if (j >= 0) {
        // no matches
        if (it != NULL) str_finish_it(*a);
        *a = NULL;
        return;
    }

    if (it != NULL) it->free_data(&it->starting_char);

    StrIt_Impl out_it = s_malloc(sizeof(struct _str_it_t) + sizeof(char*) * (data->data.ref_len + 1));
    HANDLE_OOM_VOID(out_it);
    out_it->find_data = data;
    out_it->free_data = find_data_free;
    out_it->start_index = i + 1;
    out_it->end_index = i + 1 + data->data.ref_len;
    out_it->move_next = find_move_next;
    memcpy(&out_it->starting_char, &data->data.original->starting_char, data->data.ref_len);
    (&out_it->start_index)[data->data.ref_len] = '\0';
    //*a = out_it;
}

// Boyer Moore algo
StrIt str_find(Str src, char *find, size_t find_len) {
    StrFindData data = s_malloc(sizeof(struct _str_find_data_t));
    HANDLE_OOM(data);
    data->data.ref = find;
    data->data.ref_len = find_len;
    data->data.original = convert_from_str(src);

    /* delta 1, relate each character to its last occurence in the pattern */
    for (int i = 0; i < ALPHABET_LEN; i++) {
        data->delta1[i] = find_len;
    }

    for (int i = 0; i < find_len - 1; i++) {
        data->delta1[find[i]] = find_len - 1 - i;
    }

    /* delta 2, find the minimum amount we can shift */
    // prefix pattern
    data->delta2 = malloc(sizeof(int) * find_len);
    int last_prefix = 1;
    for (int i = find_len - 1; i >= 0; i--) {
        if (suffix_is_prefix(find, find_len, i + 1)) {
            last_prefix = i + 1;
        }
        data->delta2[i] = find_len - i + last_prefix;
    }

    // suffix pattern, this will just help in some cases to avoid incorrect jumps
    for (int i = 0; i < find_len - 1; i++) {
        // find suffix length
        int s_len = 0;
        while (find[i - 1] == find[find_len - 1 - i] && s_len < i) s_len++;
        if (find[i - s_len] != find[find_len - 1 - s_len]) {
            data->delta2[find_len - 1 - s_len] = find_len - 1 - i + s_len;
        }
    }

    StrIt a;
    find_next(NULL, &a, data);
    return a;
}

Str str_replace(char *src, char *replace, char *with, size_t src_len) {
    size_t replace_len = strlen(replace);
    size_t with_len = strlen(with);
    if (replace_len == with_len) {
        // no need for more memory
        Str_Impl str = convert_from_str(str_new_prepare(src_len));
        memcpy(&str->starting_char, src, src_len);
        for (size_t i = 0; i < src_len; i++) {
            
        }
    } else if (replace_len < with_len) {
        // more complex version
    } else {
        // simpler we just have to truncate the space
    }

    return NULL;
}

Str strn_replace(Str src, char *replace, char *with) {
    return str_replace(src, replace, with, str_len(src));
}

void str_append_char(Str *a, char c) {
    // Str_Impl str = convert_from_str(*a);
    // size_t old_len = str->length;
    // if (str->length + 1 < str->max_length) {
    //     // fit in string
    //     str->length++;
    // } else {
    //     // extend string
    //     // idk about this, feels weird to realloc such a small amount
    //     str_prepare(a, str->max_length + 1);
    //     str = convert_from_str(*a);
    // }
    // /* == this being unrolled == */
    // (&str->starting_char)[str->length - 1] = c;
    // (&str->starting_char)[str->length] = '\0';
    // *a = &str->starting_char;

    // The above could be more efficient, I really doubt it though
    // loop unrolling would be the only thing I could think it would improve on
    // this is cleaner though, maybe test if < 5% difference I prefer this.
    static char buf[2] = {0};
    buf[0] = c;
    append_strings(a, buf, 1);
}

char *str_extract(Str a) {
    Str_Impl str = convert_from_str(a);
    char *out = malloc(sizeof(char) * (str->length + 1));
    memcpy(out, a, str->length + 1);
    return out;
}
