#ifndef ERR_TYPE_H
#define ERR_TYPE_H

#include "err.h"

// @TODO: more descriptive errors
ERR_ENUM(dict_err, DICTIONARY_FAILURE)
ERR_ENUM(std_err, STD_OOM)
ERR_ENUM(io_err, IO_FILE_INVALID)
ERR_ENUM(parse_err, PARSE_STR_MISSING_END, PARSE_ID_INVALID_CHAR)
ERR_ENUM(tokenizer_err, UNEXPECTED_EOF)
ERR_ENUM_GROUP(all_errors, io_err, std_err, parse_err, tokenizer_err, dict_err)

#endif