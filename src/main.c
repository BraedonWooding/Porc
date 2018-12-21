#include <stdio.h>

#include "tokenizer.h"
#include "err_type.h"
#include "str.h"

/* this is an example of a custom error handler for when we want nicer crash messages */
// char *get_error_name(int err) {
//     if (err == IO_FILE_INVALID) {
//         return "File not found";
//     } else {
//         return "Unknown Error";
//     }
// }

WRAP_MAIN(main_handler, all_errors) {
    char *fname = argc > 1 ? argv[1] : "test.txt";

    FILE *fp = fopen(fname, "r");
    GUARD(fp != NULL, IO_FILE_INVALID);

    Tokenizer tok = TRY(tokenize_file_stream(fp), Tokenizer);
    token next = *TRY(tokenizer_next(tok), Token);
    while (next.TokenType != TOK_EOF && next.TokenType != TOK_UNDEFINED) {
        printf("(%s) %s\n", token_to_name(next.TokenType), token_to_str(next));
        next = *TRY(tokenizer_next(tok), Token);
    }

    if (next.TokenType == TOK_UNDEFINED) {
        printf("ERROR: Undefined token\n");
    }

    fclose(fp);

    return OK(0);
}
