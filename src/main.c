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
    FILE *fp = fopen("test.txt", "r");
    Tokenizer tok = TRY(tokenize_file_stream(fp), Tokenizer);
    token next = *TRY(tokenizer_next(tok), Token);
    while (next.TokenType != TOK_EOF && next.TokenType != TOK_UNDEFINED) {
        puts(token_to_str(next));
        next = *TRY(tokenizer_next(tok), Token);
    }

    if (next.TokenType == TOK_UNDEFINED) {
        printf("ERROR: Undefined token\n");
    }

    fclose(fp);

    return OK(0);
}
