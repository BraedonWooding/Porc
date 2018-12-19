/* A semi-simple tokenizer */

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>

#include "err.h"
#include "err_type.h"
#include "obsidian.h"

typedef struct _token_t {
    union {
        char *str_lit;
        double flt_lit;
        long int_lit;
        char *str_rep;
        char char_lit;
    };

    enum {
#include "token_list.incl"
        NUM_TOKENS,
    } TokenType;
} token;

const char *token_to_name(int token_type);
const char *token_to_str(int token_type);

typedef struct _tokenizer_t *Tokenizer;

token tokenizer_get_current(Tokenizer tok);
Result(Tokenizer) tokenize_file_stream(FILE *fp);
Result(token) tokenizer_next(Tokenizer tok);
void tokenizer_push(Tokenizer tok, token to_push);

#endif
