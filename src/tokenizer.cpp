#include "tokenizer.hpp"

#include <string>

#include "token_data.incl"

Tokenizer Tokenizer::Copy() const {
    FILE *copy = fdopen(fileno(this->fp), "r");
    fpos_t pos;
    fgetpos(this->fp, &pos);
    fsetpos(copy, &pos);
    Tokenizer copy_tok = *this;
    copy_tok.fp = copy;
    return copy_tok;
}

void Tokenizer::ReadMore() {
    this->read_size = fread(this->read_buf, sizeof(char), BUF_SIZE, this->fp);
    this->cur_index = 0;
    this->read_buf[BUF_SIZE] = '\0';
}

void Tokenizer::ReadMorePreserving(size_t index, size_t len) {
    memcpy(this->read_buf, this->read_buf + index, len);
    this->read_size = fread(this->read_buf + len, sizeof(char), BUF_SIZE - len, this->fp);
    this->cur_index = len;
    this->read_buf[BUF_SIZE] = '\0';
}

Tokenizer::Tokenizer(FILE* &fp) {
    Tokenizer tok;
    this->return_current = false;
    this->fp = fp;
    this->ReadMore(); // initial read
}

std::string Tokenizer::ParseString() {
    char *buf = &this->read_buf[this->cur_index];
    // obs_assert(buf[0], ==, '"');
    // adjust for "
    buf++;
    size_t len = this->read_size - this->cur_index - 1;
    int i = 0;
    bool escaped = false;
    std::string str = std::string();

    for (;;) {
        // in some cases we may need to grab more from file
        // for big strings or awkard places where the string is
        // towards the end of the 1024 boundary
        if (i == len) {
            this->ReadMore();
            // check we aren't EOF
            // GUARD(this->read_size > 0, PARSE_STR_MISSING_END, str_free(str));
            // reset
            i = 0;
            buf = this->read_buf;
            len = this->read_size;
        }
        if (buf[i] == '\\') {
            escaped = true;
        } else if (buf[i] == '"' && !escaped) {
            break;
        } else {
            escaped = false;
        }
        if (!escaped) {
            str.push_back(buf[i]);
        }
        i++;
    }

    this->cur_index += i + 2;
    // GUARD(i < len && buf[i] == '"', PARSE_STR_MISSING_END, str_free(str));
    return str;
}

bool valid_char(char c) {
    return (c >= '0' && c <= '9') || // digit
            c == 'e' || c == 'E'  || // exp
            c == '.' || c == '_';    // dec pt and seperator
}

/*
    @NOTE: future calls to this invalidates the return value so you should make
    a copy of the okay value / error value if needed (i.e. just dereference it).
*/
Token Tokenizer::ParseNumber() {
    Token ret;
    char *buf = &this->read_buf[this->cur_index];
    // We may want to set an initial size??  Would make appending easier
    // especially since we don't have growth factors yet which makes appending really expensive
    std::string str = std::string();
    int i = 0;
    size_t len = this->read_size - this->cur_index;
    bool handled_exp = false;
    bool handled_dot = false;

    // sign is separate and not handled here
    // just parsing a sequence of digits
    for (;;) {
        if (i >= len) {
            this->ReadMore();
            if (this->read_size == 0) {
                // EOF is fine
                break;
            }
            // reset
            i = 0;
            buf = this->read_buf;
            len = this->read_size;
        }

        if (!valid_char(buf[i])) break;

        // ignoring the '_' and just not adding it
        // @TODO: we want to only allow one seperator
        if (buf[i] >= '0' && buf[i] <= '9') {
            str.push_back(buf[i]);
        } else if (buf[i] == '.') {
            if (handled_dot) break;
            handled_dot = true;
            str.push_back(buf[i]);
        } else if (buf[i] == 'e' || buf[i] == 'E') {
            if (handled_exp) break;
            handled_exp = true;
            str.push_back(buf[i]);
            if (buf[i + 1] == '+' || buf[i + 1] == '-') {
                i++;
                str.push_back(buf[i]);
            }
        }
        i++;
    }
    this->cur_index += i;

    // @TODO: 0.e3 is ambiguous could be calling method/property/field e3 on integer 0
    // or could be just referring to the exponent being 3 i.e. equivalent to 0e3 or 0.0e3
    // we should probably raise an error for this case forcing them to either do 0e3 0.0e3
    // if wanting the exponent or (0).e3 if wanting the call.  Applies to E as well.
    // The reason why i'm not dealing with this now is idk if I want member accesses to occur
    // on values (other than tuple sets).
    if (handled_dot || handled_exp) {
        ret.type = TokenType::Flt;
        ret.flt_lit = stof(str);
    } else {
        ret.type = TokenType::Int;
        ret.int_lit = stoi(str);
    }
    return ret;
}

Token Tokenizer::GetCurrent() const {
    return this->current_token;
}

bool IsWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool IsWhiteSpaceStr(char *c) {
    return IsWhitespace(*c);
}

void Tokenizer::ReadWhile(FnReadTillPredicate predicate) {
    if (this->read_size == 0) return;
    while (predicate(&this->read_buf[this->cur_index])) {
        this->cur_index++;
        if (this->cur_index == this->read_size) {
            this->ReadMore();
        }
        if (this->read_size == 0) return;
    }
}

std::string Tokenizer::ReadWhileGrabbingText(FnReadTillPredicate predicate) {
    std::string str = std::string();
    if (this->read_size == 0) return str;
    while (predicate(&this->read_buf[this->cur_index])) {
        str.push_back(this->read_buf[this->cur_index]);
        this->cur_index++;
        if (this->cur_index == this->read_size) this->ReadMore();
        if (this->read_size == 0) return str;
    }
    return str;
}

bool valid_starting_num(char *c) {
    return (*c >= '0' && *c <= '9') || ((*c == '_' || *c == '.') && valid_starting_num(c + 1));
}

bool valid_id_char_starting(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool valid_id_char(char c) {
    return valid_id_char_starting(c) || (c >= '0' && c <= '9');
}

bool not_end_block_comment(char *c) {
    // not very robust a bit hacky
    return (*c != '*' && c[1] != '/');
}

bool not_newline(char *c) {
    return *c != '\n';
}

Token Tokenizer::ParseIdentifier() {
    Token ret;
    std::string str = std::string();
    // GUARD(valid_id_char_starting(this->read_buf[this->cur_index]), PARSE_ID_INVALID_CHAR);
    str.push_back(this->read_buf[this->cur_index++]);

    while (valid_id_char(this->read_buf[this->cur_index])) {
        str.push_back(this->read_buf[this->cur_index++]);
    }
    ret.type = TokenType::Identifier;
    ret.str_lit = strdup(str.c_str());
    return ret;
}

// parses token and edits this->current_token.s
// returning it
Token Tokenizer::ParseBuffer() {
    this->current_token.type = TokenType::Undefined;
    ReadWhile(IsWhiteSpaceStr);

    if (this->read_size == 0) {
        this->current_token.type = TokenType::EndOfFile;
        return this->current_token;
    }

    // just for easy access
    char *buf = &this->read_buf[this->cur_index];
    size_t len = this->read_size - this->cur_index;

    // parse complicated
    switch (buf[0]) {
        case '"': {
            this->current_token.type = TokenType::Str;
            this->current_token.str_lit = strdup(ParseString().c_str());
        } break;
        case '\'': {
            // character
            // @TODO
        } break;
    }

    // Parse number?
    if (valid_starting_num(buf)) this->current_token = ParseNumber();

    if (this->current_token.type != TokenType::Undefined) return this->current_token;

    const TokenSet *current_set = &tokenFromStrMap;
    const TokenSet *previous_set = NULL;
    int i = 0;

    /*
        @REFACTOR: this is just a rough sketch of how it works
                   would definitely benefit from some robustness
    */

    while (true) {
        // check if we need to read more
        if (this->cur_index + i == this->read_size) {
            // @NOTE: technically this breaks if we ever have a token that is larger
            // than the buf_size (not including strings/identifiers/numbers/...)
            // so you would need something like >>>>> (*255) to be a valid singular (not multiple)
            // token which is not going to happen (based purely on the fact of its absurdity)
            ReadMorePreserving(this->cur_index, i);
            buf = &this->read_buf[this->cur_index - 1];
            len = this->read_size;
            this->cur_index = 0;
        }

        // are we at the leaf of a trie
        if (current_set->child_tokens == NULL || (current_set->child_tokens[buf[i]].child_tokens == NULL && 
                                                  current_set->child_tokens[buf[i]].tokens == NULL)) {
            // are we a valid token
            if (current_set->tokens != NULL && current_set->tokens[buf[i]] != (int)TokenType::Undefined) {
                this->current_token.type = (TokenType)current_set->tokens[buf[i]];
                this->cur_index += i + 1;
                break;
            } else {
                // was the previous token valid (i.e. `<!` should be parsed as `<` and `!`)
                if (previous_set != NULL && previous_set->tokens != NULL && previous_set->tokens[buf[i - 1]] != (int)TokenType::Undefined) {
                    this->current_token.type = (TokenType)previous_set->tokens[buf[i - 1]];
                    // we don't want to + 1 here also because that we want to return the 
                    // previous token map not the current.
                    this->cur_index += i;
                    break;
                } else if (this->read_size == 0) {
                    // in the case where we are missing the end of a token
                    // note: we may want to change this when we introduce
                    // identifiers and further parsing
                    return Token(TokenType::Undefined);
                }
                break;
            }
        } else {
            // go deeper into set
            previous_set = current_set;
            current_set = &current_set->child_tokens[buf[i]];
            i++;
        }
    }

    if (this->current_token.type == TokenType::LineComment) {
        // read till \n
        this->current_token.str_lit = strdup(ReadWhileGrabbingText(not_newline).c_str());
    } else if (this->current_token.type == TokenType::BlockComment) {
        // read till */
        this->current_token.str_lit = strdup(ReadWhileGrabbingText(not_end_block_comment).c_str());
        this->cur_index += 2;
    }

    if (this->current_token.type == TokenType::Undefined && valid_id_char_starting(buf[0])) {
        this->current_token = ParseIdentifier();
    }

    return this->current_token;
}

const char *Token::ToString() {
    if (this->type == TokenType::Str || this->type == TokenType::Identifier) return this->str_lit;
    char *out = NULL;
    if (this->type == TokenType::LineComment) asprintf(&out, "//%s", this->str_lit);
    if (this->type == TokenType::BlockComment) asprintf(&out, "/*%s*/", this->str_lit);
    if (this->type == TokenType::Flt) asprintf(&out, "%lf", this->flt_lit);
    if (this->type == TokenType::Int) asprintf(&out, "%ld", this->int_lit);
    if (out != NULL) return out;
    if (tokenToStrMap[(int)this->type] != NULL) return tokenToStrMap[(int)this->type];
    printf(__FILE__":%d CASE NOT HANDLED\n", __LINE__);
    abort();
}

const char *Token::ToName() {
    return tokenToNameMap[(int)this->type];
}

Token Tokenizer::Next() {
    if (this->return_current) {
        this->return_current = false;
        return this->current_token;
    }

    if (this->cur_index == this->read_size) ReadMore();
    return ParseBuffer();
}

void Tokenizer::Push(Token to_push) {
    this->current_token = to_push;
    this->return_current = true;
}
