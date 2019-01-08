#include "tokenizer.hpp"

#include <string>
#include <locale>
#include <optional>
#include "helper.hpp"

#include "token_data.incl"

/*
    Helpful notes:
    This is mostly here since our nature of reading is a little odd perhaps?
    And so I don't forget what I did.

    - We have boundary reads which means that since we read in segments
      (rather than whole file into a string) we need to take care of the rare
      cases where a token is spread over one or multiple boundaries
        - In actual practice this will only effect large strings/comments
          since the chance it will hit a token is pretty small most likely it'll hit
          whitespace or an identifier which are both really cheap in terms of how
          we cope with it
            - They are only 'expensive' when we have to keep parts of the
              current buffer and can't ReadAll (still it is quite cheap)
    - We have to be wary of the boundary reads i.e. ['\']['\'] has to be passed correctly
        - A few helpful functions; Read/ReadAll, and BufMatches
        - BufMatches especially is really nice to check if the buffer contains a string
          and generally how it increments cur_index is what you want, you can always
          adjust it further if you really need to.
    - We also have to adjust cur_col/line appropriately but that is pretty easy
      and mostly covered by the parse functions.
    - We do it this way since it means a few things;
        - More efficient/Usable on memory sparce systems (low amount of ram)
        - Better IO pipelining (consistently small reads is better than
          one large read).
        - Supports insanely large files better
        - Doesn't cause huge segmentation (this really only effects interpreters which we are)
            - This is because if we allocate a huge memory block before we are even running
              the program then deallocating before we run it we could possibly cause some
              segmentation (of course in reality large blocks of segmentation are rarely an issue)

    @NOTE:  make sure you are using fadvise (FADV_NOREUSE | FADV_SEQUENTIAL)
            to get the most performance out of this way of reading the files.
*/

TokenStream::TokenStream(std::unique_ptr<Reader> reader): reader(std::move(reader)) {
    this->cur.type = TokenType::Undefined;
    this->ret_cur = false;
    this->cur_index = this->read_size = this->cur_line = this->cur_col = 0;
}

void TokenStream::ReadAll() {
    Read(BUF_SIZE, 0);
}

bool TokenStream::IsDone() const {
    return read_size == 0 && !ret_cur;
}

void TokenStream::Push(Token tok) {
    if (this->ret_cur) throw __FILE__":Tokenizer::Push can't push onto token stream with pushed token";
    this->cur = tok;
    this->ret_cur = true;
}

Token TokenStream::Next() {
    if (this->ret_cur) {
        this->ret_cur = false;
        return this->cur;
    }

    if (this->cur_index == this->read_size) ReadAll();
    if (this->read_size == 0) return this->cur = Token::EndOfFile();
    return Parse();
}

void TokenStream::SkipWs() {
    while(this->read_size != 0 && std::isspace(this->read_buf[this->cur_index])) {
        this->cur_index++, this->cur_col++;
        if (this->cur_index == this->read_size) ReadAll();
        if (this->read_buf[this->cur_index] == '\n') this->cur_line++, this->cur_col = -1;
    }
}

std::string TokenStream::ParseLineComment() {
    std::string buf;
    while (true) {
        if (this->cur_index == this->read_size) ReadAll();

        if (this->read_size == 0 || this->read_buf[this->cur_index] == '\n') break;
        buf.push_back(this->read_buf[this->cur_index++]);
    }
    return buf;
}

bool TokenStream::BufMatches(std::string str) {
    for (int i = 0; i < str.size(); i++) {
        if (this->cur_index == this->read_size) ReadAll();
        if (this->read_size == 0) return false;

        if (str[i] != this->read_buf[this->cur_index]) return false;
        this->cur_index++;
    }
    return true;
}

// supports nesting
std::optional<std::string> TokenStream::ParseBlockComment() {
    std::string buf;
    int depth = 1;

    while (depth > 0) {
        if (this->cur_index == this->read_size) ReadAll();
        // block comment not ended
        if (this->read_size == 0) return std::nullopt;
        if (this->read_buf[this->cur_index] == '*') {
            if (BufMatches("*/")) {
                depth--;
                continue;
            } else {
                this->cur_index--;
            }
        } else if (this->read_buf[this->cur_index] == '/') {
            if (BufMatches("/*")) {
                depth++;
                continue;
            } else {
                this->cur_index--;
            }
        }
        buf.push_back(this->read_buf[this->cur_index++]);
    }
    return buf;
}

void TokenStream::ParseSimpleToken() {
    char *buf = &this->read_buf[this->cur_index];
    size_t len = this->read_size - this->cur_index;

    // @OPTIMISATION: these will be more common so maybe we should put these above the others
    // or we should see if we can jump straight to the first token easily
    const TokenSet *current_set = &tokenFromStrMap;
    const TokenSet *previous_set = NULL;
    int i = 0;

    // Recursively call on each child token set till we reach a dead end
    // either we can go back one level and step into the 'value' section of the tree
    // or we can't and thus no token can be found. i.e. `+>` will parse the `+` and `>` separately
    // `+=>` will parse as `+=` and `>`.  Of course this may not be preferred so we may want to
    // have a precendence for tokens, I think it is fine though for now.
    while (true) {
        if (this->cur_index + i == this->read_size) {
            Assert(this->cur_index != 0, "We can't have a token with a length more than buf_size", this->cur_index);
            // This is incase I ever change things and break this offcase
            // We should never get to this point and to not have iterated i
            // Since that would mean that our buf is full which can't be the case at this pt
            // if we have not read using i.
            Assert(i > 0, "Buffer shouldn't be full if i == 0", i);

            // shuffle the important bits down the the front of the buffer
            memmove((char*)this->read_buf, (char*)this->read_buf + this->cur_index, i);

            // now we can read into the buffer
            Read(this->read_size - i, i);
            buf = this->read_buf;
            len = this->read_size;

            if (this->read_size == 0) {
                // Our token wasn't finished
                // @TEST: This may trigger incorrectly if the last token is something like
                // `cont` or similar.
                // @POSSIBLE_FIX: I think I have fixed it as the identifier will grab
                // it if it can be regarded as an identifier/number, again test/check
                this->read_buf[i + 1] = 0;
                std::cerr << "Possibly incorrect trigger read text so far is " << this->read_buf + this->cur_index << std::endl;
                this->cur = Token(TokenType::Undefined, LineRange(this->cur_line, this->cur_line, this->cur_col, this->cur_col + i));
            }
        }

        // trie leaf
        // @NOTE: you can't have WS in tokens that is a token can't contain a line break
        // or any kinda of ws in the middle of it i.e = > is always going to be `=` and `>` and never `=>`.
        // This statement allows us to just increment cur_col as expected.
        if (current_set->child_tokens == NULL || (current_set->child_tokens[buf[i]].child_tokens == NULL && 
                                                  current_set->child_tokens[buf[i]].tokens == NULL)) {
            // We hit a dead end but can we find a value node either in our current node
            // or go back one token
            if (current_set->tokens != NULL && current_set->tokens[buf[i]] != (int)TokenType::Undefined) {
                this->cur = Token((TokenType)current_set->tokens[buf[i]], LineRange(this->cur_line, this->cur_line, this->cur_col, this->cur_col + i));
                this->cur_index += i + 1;
                this->cur_col += i + 1;
            } else {
                // i.e. parse `<!` as `<` and `!`
                if (previous_set != NULL && previous_set->tokens != NULL && previous_set->tokens[buf[i - 1]] != (int)TokenType::Undefined) {
                    this->cur = Token((TokenType)previous_set->tokens[buf[i - 1]], LineRange(this->cur_line, this->cur_line, this->cur_col, this->cur_col + i - 1));
                    this->cur_index += i;
                    this->cur_col += i;
                }
            }
            // we want to just fall through if we don't get a nice token
            break;
        } else {
            // go deeper into set
            previous_set = current_set;
            current_set = &current_set->child_tokens[buf[i]];
            i++;
        }
    }
}

void TokenStream::ParseChar() {
    Unreachable("TODO");
}

Token TokenStream::Parse() {
    this->cur.type = TokenType::Undefined;
    SkipWs();

    // After skipping WS we at EOF
    if (this->read_size == 0) return this->cur = Token::EndOfFile();

    // parse complicated tokens
    switch (this->read_buf[this->cur_index]) {
        case '"': ParseStr(); break;
        case '\'': ParseChar(); break;
        default: ParseNum(); break;
    }

    if (this->cur.type != TokenType::Undefined) return this->cur;
    ParseSimpleToken();

    if (this->cur.type == TokenType::LineComment) {
        // read till \n
        this->cur.data = ParseLineComment();
    } else if (this->cur.type == TokenType::BlockComment) {
        // read till */
        uint old_line = cur_line;
        uint old_col = cur_col;
        auto comment = ParseBlockComment();
        if (!comment) {
            // missing `*/`
            return this->cur = Token(TokenType::Undefined, LineRange(old_line, cur_line, old_col, cur_col));
        }
        this->cur.data = comment.value();
    }

    if (this->cur.type == TokenType::Undefined) ParseId();
    return this->cur;
}

Token TokenStream::GetCurrent() const {
    return this->cur;
}

std::optional<ulong> TokenStream::ParseSimpleNumber(int max_digits, int base) {
    std::string cur;
    int cur_digits = 0;

    while (cur_digits < max_digits) {
        if (cur_index == read_size) ReadAll();
        // I guess this could be valid (though I don't see how since it won't have a string ender)
        // Still should be rigorous I guess
        if (read_size == 0) break;
        if (read_buf[cur_index] >= '0' && read_buf[cur_index] <= '9') {
            cur_digits++;
            cur.push_back(read_buf[cur_index]);
            cur_index++;
        } else {
            // reached invalid char
            // (definitely can be a viable string for example \na is just `\n` and `a`)
            // @INFO: We could support `_` in numbers here too????
            break;
        }
    }

    if (cur_digits == 0) return std::nullopt;
    return std::stoi(cur, 0, base);
}

// returns str since we may return multiple unicode chars for a single codepoint
void TokenStream::ConvEscapeCodes(std::string &str) {
    switch(read_buf[cur_index]) {
        case 'n': str.push_back('\n'); break;
        case 'r': str.push_back('\r'); break;
        case 't': str.push_back('\t'); break;
        case 'v': str.push_back('\v'); break;
        case 'a': str.push_back('\a'); break;
        case 'b': str.push_back('\b'); break;
        case 'f': str.push_back('\f'); break;
        case '0': case 'x': {
            // hex
            if (read_buf[cur_index] == '0') {
                if (!BufMatches("0x")) {
                    // we are an octal
                    // has to be duplicated since we have to cover the 'else' case differently
                    auto num = ParseSimpleNumber(3, 8);
                    str.push_back(num ? static_cast<unsigned char>(num.value()) : 0);
                    return;
                }
            } else {
                cur_index++;
            }
            auto num = ParseSimpleNumber(2, 16);
            if (num) str.push_back(static_cast<unsigned char>(num.value()));
            else {
                // @TODO: error
                std::cerr << "Invalid str lit!!!  TODO: better errors" << std::endl;
            }
            return;
        } break;
        case 'u': {
            // small unicode
            // @TODO: (I would like a stdlib way the other way is ugh)
            Unreachable("Unreachable, TODO");
        } break;
        case 'U': {
            // large unicode
            // @TODO: (I would like a stdlib way the other way is ugh)
            Unreachable("Unreachable, TODO");
        } break;
        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
            auto num = ParseSimpleNumber(3, 8);
            if (num) str.push_back(static_cast<unsigned char>(num.value()));
            else {
                // @TODO: error
                std::cerr << "Invalid str lit!!!  TODO: better errors" << std::endl;
            }
            return;
        } break;
        default: str.push_back(read_buf[cur_index]);
    }
    // for the first set of things
    cur_index++;
}

void TokenStream::ParseStr() {
    char *buf = &this->read_buf[this->cur_index];
    Assert(buf[0] == '"', "Expecting buf to begin with \"", buf[0]);

    buf++;
    size_t len = this->read_size - this->cur_index - 1;
    int i = 0;
    bool escaped = false;
    std::string str = std::string();
    uint old_line = cur_line;
    uint old_col = cur_col;

    for (;;) {
        // in some cases we may need to grab more from file
        // for big strings or awkard places where the string is
        // towards the end of the 1024 boundary
        if (i == len) {
            ReadAll();
            if (this->read_size == 0) {
                // if we are EOF then just set the undefined token and ret
                this->cur.type = TokenType::Undefined;
                return;
            }

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
            if (!escaped)   str.push_back(buf[i]);
            else            ConvEscapeCodes(str);
            escaped = false;
        }
        i++;
    }

    // +2 since +1 for each `"`
    this->cur_col += this->cur_index += i + 2;
    this->cur = Token(TokenType::Str, LineRange(old_line, cur_line, old_col, cur_col - 1), str);
}

bool valid_char(char c) {
    return (c >= '0' && c <= '9') || // digit
            c == 'e' || c == 'E'  || // exp
            c == '.' || c == '_';    // dec pt and seperator
}

bool is_num(char c) {
    return c >= '0' && c <= '9';
}

void TokenStream::ParseNum() {
    char *buf = &this->read_buf[this->cur_index];

    /*
        @TEST:
        Not a number (flt/int) if:
        1) first char is not a digit, not 'e' not 'E' not '.'
        4) first char is '.' second char not a digit
        Note: Not allowed `_` to avoid any of the cases above
    */

    // Super ugly, should be hoistered into callsite
    // or we should just exit gracefully???
    if (!is_num(buf[0])) {
        if (cur_index >= read_size - 2) {
            // since we need at most three to see
            // should be rare enough that this doesn't matter
            Read(BUF_SIZE - 2, cur_index + 2);
            buf = read_buf;
            cur_index = 0;
        }

        if (buf[0] != '.' || !is_num(buf[1])) {
            return;
        }
    }

    std::string str = std::string();
    int i = 0;
    size_t len = this->read_size - this->cur_index;
    bool handled_exp = false;
    bool handled_dot = false;
    bool prev_exp = false;

    // sign is separate and not handled here
    // just parsing a sequence of digits
    for (;;) {
        if (i >= len) {
            ReadAll();
            // EOF is valid so just break
            if (this->read_size == 0) break;
            // reset
            i = 0;
            buf = this->read_buf;
            len = this->read_size;
        }

        if (prev_exp) {
            if (buf[i] == '_') continue; // skip `_`

            if (buf[i] == '+' || buf[i] == '-' || is_num(buf[i])) {
                str.push_back(buf[i]);
                prev_exp = false;
                handled_exp = true;
                continue;
            } else {
                i--;
                break;
            }
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
            prev_exp = true;
            str.push_back(buf[i]);
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
        cur.type = TokenType::Flt;
        cur.data = std::stof(str);
    } else {
        cur.type = TokenType::Int;
        cur.data = static_cast<std::int64_t>(std::stol(str));
    }
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

void TokenStream::ParseId() {
    if (!valid_id_char_starting(read_buf[this->cur_index])) return;

    std::string str = std::string();
    str.push_back(this->read_buf[this->cur_index++]);

    while (valid_id_char(this->read_buf[this->cur_index])) {
        str.push_back(this->read_buf[this->cur_index++]);
    }
    cur.type = TokenType::Identifier;
    cur.data = str;
}

void TokenStream::Read(uint len, uint offset) {
    read_size = reader->Read(static_cast<char*>(read_buf) + offset, len);
    cur_index = offset;
    read_buf[offset + len] = '\0';
}

std::string Token::ToString() {
    if (this->type == TokenType::Str || this->type == TokenType::Identifier) return std::get<std::string>(this->data);
    char *out = NULL;
    if (this->type == TokenType::LineComment) {
        return std::string("//").append(std::get<std::string>(this->data));
    }
    if (this->type == TokenType::BlockComment) {
        return std::string("/*").append(std::get<std::string>(this->data)).append("*/");
    }
    if (this->type == TokenType::Flt) return std::to_string(std::get<double>(this->data));
    if (this->type == TokenType::Int) return std::to_string(std::get<std::int64_t>(this->data));
    if (out != NULL) return out;
    if (tokenToStrMap[(int)this->type] != NULL) return std::string(tokenToStrMap[(int)this->type]);

    Unreachable("Case not handled");
}

const char *Token::ToName() {
    return tokenToNameMap[(int)this->type];
}

uint CFileReader::Read(char *buf, uint len) {
    return fread(buf, sizeof(char), len, fp);
}
