/* A semi-simple tokenizer */

#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#define BUF_SIZE 1024
#define ASCII_SET 256

#include <iostream>

enum class TokenType {
#include "token_list.incl"
    NUM_TOKENS,
};

struct Token {
public:
    union {
        const char *str_lit;
        double flt_lit;
        long int_lit;
        char char_lit;
    };

    TokenType type;

    const char *ToName();
    const char *ToString();

    Token(TokenType type) {
        this->type = type;
    }

    Token() = default;
};

using FnReadTillPredicate = std::function<bool(char *c)>;

class Tokenizer {
public:
    Token current_token;
    bool return_current;
    FILE *fp;
    char read_buf[BUF_SIZE + 1];
    size_t cur_index;
    size_t read_size;

    Tokenizer(FILE* &fp);
    Token GetCurrent() const;
    Token Next();
    void Push(Token tok);
    Tokenizer Copy() const;

private:
    void ReadMore();
    void ReadMorePreserving(size_t index, size_t len);
    std::string ReadWhileGrabbingText(FnReadTillPredicate predicate);
    std::string ParseString();
    Token ParseNumber();
    void ReadWhile(FnReadTillPredicate predicate);
    Token ParseIdentifier();
    Token ParseBuffer();

    Tokenizer() = default;
};

#endif
