#include "tokenizer.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "err.h"
#include "str.h"
#include "err_type.h"
#include <string.h>

#define BUF_SIZE 1024
#define ASCII_SET 256

#include "token_data.incl"

struct _tokenizer_t {
    token current_token;
    bool return_current;
    FILE *fp;
    char read_buf[BUF_SIZE + 1];
    size_t cur_index;
    size_t read_size;
};

static void read_more(Tokenizer tok) {
    tok->read_size = fread(tok->read_buf, sizeof(char), BUF_SIZE, tok->fp);
    tok->cur_index = 0;
    tok->read_buf[BUF_SIZE] = '\0';
}

static void read_more_preserving(Tokenizer tok, size_t index, size_t len) {
    obs_assert(len, <, (size_t)BUF_SIZE);
    memcpy(tok->read_buf, tok->read_buf + index, len);
    tok->read_size = fread(tok->read_buf + len, sizeof(char), BUF_SIZE - len, tok->fp);
    tok->cur_index = len;
    tok->read_buf[BUF_SIZE] = '\0';
}

Result(Tokenizer) tokenize_file_stream(FILE *fp) {
    GUARD(fp != NULL, IO_FILE_INVALID)
    Tokenizer tok = malloc(sizeof(struct _tokenizer_t));
    GUARD(tok != NULL, STD_OOM)
    tok->return_current = false;
    tok->fp = fp;
    read_more(tok); // initial read
    return OK(tok);
}

Result(token) parse_string(Tokenizer tok) {
    char *buf = &tok->read_buf[tok->cur_index];
    obs_assert(buf[0], ==, '"');
    // adjust for "
    buf++;
    size_t len = tok->read_size - tok->cur_index - 1;
    int i = 0;
    bool escaped = false;
    Str str = str_empty();

    for (;;) {
        // in some cases we may need to grab more from file
        // for big strings or awkard places where the string is
        // towards the end of the 1024 boundary
        if (i == len) {
            read_more(tok);
            // check we aren't EOF
            GUARD(tok->read_size > 0, PARSE_STR_MISSING_END, str_free(str));
            // reset
            i = 0;
            buf = tok->read_buf;
            len = tok->read_size;
        }
        if (buf[i] == '\\') {
            escaped = true;
        } else if (buf[i] == '"' && !escaped) {
            break;
        } else {
            escaped = false;
        }
        if (!escaped) {
            str_append_char(&str, buf[i]);
        }
        i++;
    }

    tok->cur_index += i + 2;
    GUARD(i < len && buf[i] == '"', PARSE_STR_MISSING_END, str_free(str));
    return OK(str);
}

bool valid_char(char c) {
    return (c >= '0' && c <= '9') || // digit
            c == 'e' || c == 'E'  || // exp
            c == '.' || c == '_';    // dec pt and seperator
}

Result(token) parse_number(Tokenizer tok) {
    char *buf = &tok->read_buf[tok->cur_index];
    // We may want to set an initial size??  Would make appending easier
    // especially since we don't have growth factors yet which makes appending really expensive
    Str str = str_empty();
    int i = 0;
    size_t len = tok->read_size - tok->cur_index;
    bool handled_exp = false;
    bool handled_dot = false;

    // sign is separate and not handled here
    // just parsing a sequence of digits
    for (;;) {
        if (i >= len) {
            read_more(tok);
            if (tok->read_size == 0) {
                // EOF is fine
                break;
            }
            // reset
            i = 0;
            buf = tok->read_buf;
            len = tok->read_size;
        }

        if (!valid_char(buf[i])) break;

        // ignoring the '_' and just not adding it
        // @TODO: we want to only allow one seperator
        if (buf[i] >= '0' && buf[i] <= '9') {
            str_append_char(&str, buf[i]);
        } else if (buf[i] == '.') {
            if (handled_dot) break;
            handled_dot = true;
            str_append_char(&str, buf[i]);
        } else if (buf[i] == 'e' || buf[i] == 'E') {
            if (handled_exp) break;
            handled_exp = true;
            str_append_char(&str, buf[i]);
            if (buf[i + 1] == '+' || buf[i + 1] == '-') {
                i++;
                str_append_char(&str, buf[i]);
            }
        }
        i++;
    }
    tok->cur_index = i;

    // @TODO: 0.e3 is ambiguous could be calling method/property/field e3 on integer 0
    // or could be just referring to the exponent being 3 i.e. equivalent to 0e3 or 0.0e3
    // we should probably raise an error for this case forcing them to either do 0e3 0.0e3
    // if wanting the exponent or (0).e3 if wanting the call.  Applies to E as well.
    // The reason why i'm not dealing with this now is idk if I want member accesses to occur
    // on values (other than tuple sets).
    token token;
    if (handled_dot || handled_exp) {
        token.TokenType = TOK_FLT;
        token.flt_lit = atof(str);
    } else {
        token.TokenType = TOK_INT;
        token.int_lit = atoi(str);
    }
    str_free(str);
    return OK(token);
}

Token tokenizer_get_current(Tokenizer tok) {
    return &tok->current_token;
}

bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

void skip_ws(Tokenizer tok) {
    if (tok->read_size == 0) return;
    while (is_whitespace(tok->read_buf[tok->cur_index])) {
        tok->cur_index++;
        if (tok->cur_index == tok->read_size) {
            read_more(tok);
        }
        if (tok->read_size == 0) return;
    }
}

// parses token and edits tok->current_token.s
// returning it
Result(Token) parse_buf(Tokenizer tok) {
    tok->current_token.TokenType = TOK_UNDEFINED;
    skip_ws(tok);

    if (tok->read_size == 0) {
        tok->current_token.TokenType = TOK_EOF;
        return OK(&tok->current_token);
    }

    // just for easy access
    char *buf = &tok->read_buf[tok->cur_index];
    size_t len = tok->read_size - tok->cur_index;
    TokenSet current_set = &tokenFromStrMap;
    TokenSet previous_set = NULL;
    int i = 0;

    /*
        @REFACTOR: this is just a rough sketch of how it works
                   would definitely benefit from some robustness
    */

    while (true) {
        // check if we need to read more
        if (tok->cur_index == tok->read_size) {
            // @NOTE: technically this breaks if we ever have a token that is larger
            // than the buf_size (not including strings/identifiers/numbers/...)
            // so you would need something like >>>>> (*255) to be a valid singular (not multiple)
            // token which is not going to happen (based purely on the fact of its absurdity)
            read_more_preserving(tok, tok->cur_index - i, i);
            buf = &tok->read_buf[tok->cur_index - 1];
            len = tok->read_size - tok->cur_index;
        }

        // are we at the leaf of a trie
        if (current_set->child_tokens == NULL || current_set->child_tokens[buf[i]] == NULL) {
            // are we a valid token
            if (current_set->tokens[buf[i]] != TOK_UNDEFINED) {
                tok->current_token.TokenType = current_set->tokens[buf[i]];
                tok->cur_index++;
                return OK(&tok->current_token);
            } else {
                // was the previous token valid (i.e. `<!` should be parsed as `<` and `!`)
                if (previous_set != NULL && previous_set->tokens[buf[i - 1]] != TOK_UNDEFINED) {
                    tok->current_token.TokenType = previous_set->tokens[buf[i - 1]];
                    tok->cur_index++;
                    return OK(&tok->current_token);
                } else if (tok->read_size == 0) {
                    // in the case where we are missing the end of a token
                    // note: we may want to change this when we introduce
                    // identifiers and further parsing
                    return ERR(UNEXPECTED_EOF);
                }
                break;
            }
        } else {
            // go deeper into set
            previous_set = current_set;
            current_set = current_set->child_tokens[buf[i]];
            tok->cur_index++;
        }
        i++;
    }

    // parse complicated tokens
    switch (buf[0]) {
        case '"': {
            Str out = TRY(parse_string(tok), Str);
            tok->current_token.TokenType = TOK_STR;
            tok->current_token.str_lit = str_extract(out);
            str_free(out);
        } break;
        case '\'': {
            // character
            // @TODO
        } break;
    }

    return OK(&tok->current_token);
}

const char *token_to_str(token tok) {
    if (tokenToStrMap[tok.TokenType] != NULL) return tokenToStrMap[tok.TokenType];
    if (tok.TokenType == TOK_STR) return tok.str_lit;
    printf("NOT HANDLED");
    abort();
}

const char *token_to_name(int type) {
    return tokenToNameMap[type];
}

Result(Token) tokenizer_next(Tokenizer tok) {
    obs_assert(tok, !=, NULL);
    if (tok->return_current) {
        tok->return_current = false;
        return OK(&tok->current_token);
    }

    if (tok->cur_index == tok->read_size) read_more(tok);
    return parse_buf(tok);
}

void tokenizer_push(Tokenizer tok, token to_push) {
    tok->current_token = to_push;
    tok->return_current = true;
}
