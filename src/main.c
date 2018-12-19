#include <stdio.h>

#include "tokenizer.h"
#include "err_type.h"

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
    token next = TRY(tokenizer_next(tok), token);
    while (next.TokenType != TOK_EOF) {
        puts(token_to_str(next.TokenType));
        next = TRY(tokenizer_next(tok), token);
    }

    fclose(fp);

    return OK(0);
}
