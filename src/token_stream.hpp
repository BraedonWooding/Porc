#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <iostream>
#include <variant>
#include <string>
#include <optional>
#include "printer_helpers.hpp"
#include "reader.hpp"
#include "token.hpp"
#include "defs.hpp"

namespace porc::internals {

/*
  A token stream built for efficient lookaheads
  You can view it as being a stream with 3 different operations.
  `PopCur()` returns the top token and moves the index back.
  Upon reaching `Lookaheads` it will reallocate both.

  Example 1 (simple iteration):
    // 'reader' is some class of base type Reader.
    TokenStream stream(reader);
    // auto calls `stream.Move()`
    Token tok;

    // read all in various different ways
    // PopCur() returns the current token and pops it from the stream
    // PeekCur() doesn't do the pop
    while (tok = stream.PopCur()) { ... }
    // or using for
    for (tok = stream.PopCur(); stream.Next(); tok = stream.PopCur()) { ... }
    // another way
    for (tok = stream.PopCur(); tok; tok = stream.PopCur()) { ... }

    // at the end you may want to check if Undefined compared to EndOfFile
    if (tok.type == Token::Kind::Undefined) {
      // handle error case
    }

  You can also `Push()` onto the stream till reached Lookaheads (default 2).
*/
class TokenStream {
 private:
  static const int BufSize = 1024;
  const int MaxLookaheads = 2;

  std::vector<Token> tokens;

  /* For storing the data read into the stream */
  char read_buf[BufSize + 1];

  /* Current index into read_buf */
  uint cur_index = 0;

  /* How much was read on last read, is == 0 for EOF */
  uint read_size = 0;

  /* The current vertical height */
  uint line = 0;

  /* Current horizontal height */
  uint col = 0;

  /* The reader to read in data */
  std::unique_ptr<Reader> reader;

  // Reads len bytes into read_buf starting at read_buf + offset
  void Read(uint len, uint offset);

  // Equivalent to Read(BufSize, 0);
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
  Token ParseStr();
  Token ParseChar();
  Token ParseNum();
  Token ParseId();
  std::string ParseLineComment();

  /*
    Parses a simple token into cur.
    Postconditions: read_size == 0 || cur_index < read_size
  */
  Token ParseSimpleToken();
  std::optional<std::string> ParseBlockComment();
  void ConvEscapeCodes(std::string &str);

  // Returns true if the buffer (will read more if it crosses boundaries)
  // contains str
  // @NOTE:   cur_index will update and will point to the first
  //          character that doesn't match.
  //          you can't also guarantee that the entire string still
  //          exists within the current read buffer as it doesn't preserve
  //          the string.
  bool BufMatches(std::string str);

 public:

  /*
    Returns the current token.  Can only be called once per Next.
  */
  Token PopCur();

  /*
    Returns the current token.  Can be called endlessly.
  */
  Token PeekCur();

  /*
    Moves last into cur
  */
  bool Next();
  void Push(Token tok);

  TokenStream(std::unique_ptr<Reader> reader, int max_lookaheads=2)
      : reader(std::move(reader)), MaxLookaheads(max_lookaheads) {
    tokens.reserve(max_lookaheads);
  }
};

}

#endif
