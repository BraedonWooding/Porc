#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#define BUF_SIZE 1024
#define ASCII_SET 256

#include <iostream>
#include <variant>
#include <string>
#include <optional>
#include "printer_helpers.hpp"
#include "reader.hpp"
#include "defs.hpp"

enum class TokenType {
#include "token_list.inc"
  NUM_TOKENS,
};

struct Token {
 public:
  std::variant<std::string, double, std::int64_t, char> data;

  TokenType type;
  LineRange pos;

  const char *ToName();
  std::string ToString();

  Token() : pos(LineRange::NullRange()) {};
  Token(TokenType type, LineRange pos): pos(pos), type(type) { }

  template<typename T>
  Token(TokenType type, LineRange pos, T data): pos(pos), type(type), data(data) { }

  static Token EndOfFile() {
    return Token(TokenType::EndOfFile, LineRange::NullRange());
  }
};

/*
  Our token stream, you can go back one token if needed.
  Going back two tokens is an exception.
*/
class TokenStream {
 private:
  /* Our current Token (from the last 'Next') */
  Token cur;

  /* If true return current token on `Next` */
  bool ret_cur;

  /* For storing the data read into the stream */
  char read_buf[BUF_SIZE + 1];

  /* Current index into read_buf */
  uint cur_index;

  /* How much was read on last read, is == 0 for EOF */
  uint read_size;

  /* The current vertical height */
  uint line;

  /* Current horizontal height */
  uint col;

  /* The reader to read in data */
  std::unique_ptr<Reader> reader;

  // Reads len bytes into read_buf starting at read_buf + offset
  void Read(uint len, uint offset);

  // Equivalent to Read(BUF_SIZE, 0);
  void ReadAll();
  Token Parse();

  /*

  */
  std::optional<int> ParseSimpleNumber(int max_digits, int base);

  /*
    Skips all whitespace incrementing cur_index and reading in new segments.
    Preconditions: read_size == 0 || cur_index < read_size
    Postconditions: cur_index < read_size || read_size == 0
  */
  void SkipWs();
  void ParseStr();
  void ParseChar();
  void ParseNum();
  void ParseId();
  std::string ParseLineComment();

  /*
    Parses a simple token into cur.
    Postconditions: read_size == 0 || cur_index < read_size
  */
  void ParseSimpleToken();
  std::optional<std::string> ParseBlockComment();
  void ConvEscapeCodes(std::string &str);

  // Returns true if the buffer (will read more if it crosses boundaries) contains str
  // @NOTE:   cur_index will update and will point to the first
  //          character that doesn't match.
  //          you can't also guarantee that the entire string still
  //          exists within the current read buffer as it doesn't preserve the string.
  bool BufMatches(std::string str);

 public:
  Token GetCurrent() const;
  Token Next();
  void Push(Token tok);
  bool IsDone() const;

  TokenStream(std::unique_ptr<Reader> reader);
};

#endif
