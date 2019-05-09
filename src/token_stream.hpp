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

// @TODO: make these static constants or something idk
// I just don't want them to be #defines and I haven't decided if array
// should just be two tokens 'hard' coded.
#define MaxLookaheads (2)
#define BufSize (1024)

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
    if (tok.type == Token::Undefined) {
      // handle error case
    }

  You can also `Push()` onto the stream till reached Lookaheads (default 2).
*/
class TokenStream {
 private:
  std::array<Token, MaxLookaheads> tokens;
  int cur_token_size = 0;

  /* For storing the data read into the stream */
  char read_buf[BufSize + 1];

  /* Current index into read_buf */
  uint cur_index = 0;

  /* How much was read on last read, is == 0 for EOF */
  uint read_size = 0;

  /* The current vertical height */
  int line = 1;

  /* Current horizontal height */
  int col = 1;

  int old_line = 0;
  int old_col = 0;

  /* The reader to read in data */
  std::unique_ptr<Reader> reader;

  // Reads len bytes into read_buf starting at read_buf + offset
  void Read(uint len, uint offset);

  // Equivalent to Read(BufSize, 0);
  void ReadAll();
  Token Parse();

  // setup the old line / col
  inline void BeginLineRange();

  // finalise the position
  // col offset is used to adjust col before
  // i.e. EndLineRange(-1) == LineRange(old_line, line, old_col, col -1)
  inline LineRange EndLineRange(int col_offset = 0) const;

  /*
    NOTE: should only call this after calling a read function
          this is because for example at the very start it'll be true
          as we will have read nothing.
  */
  bool IsEOF() const { return read_size == 0; }

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

  bool ignore_comments = true;

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

  const std::string &GetFileName() const { return reader->file_name; };

  TokenStream(std::unique_ptr<Reader> reader)
      : reader(std::move(reader)), tokens{} {}
};

}

#endif
