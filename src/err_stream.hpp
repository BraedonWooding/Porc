#ifndef ERR_STREAM_HPP
#define ERR_STREAM_HPP

#include <iostream>
#include <string>

#include "token_stream.hpp"

namespace porc::internals {

template<typename T, typename E>
using expected_unique_ptr = tl::expected<std::unique_ptr<T>, E>;

template<typename T>
using optional_unique_ptr = std::optional<std::unique_ptr<T>>;

struct ParseError {
  enum class Kind {
    MissingToken, // missing a token (not EOF)
    InvalidToken, // wasn't expecting this token
    ValidEOF,     // not really an error more just info that we ran out
    InvalidEOF,   // wasn't expecting EOF
  };

public:
  Kind kind;
  std::string extra_info;
  Token related_token;

  ParseError(Kind kind) : kind(kind) {}
  ParseError(Kind kind, std::string extra_info)
      : kind(kind), extra_info(extra_info) {}
  ParseError(Kind kind, std::string extra_info, Token related_token)
      : kind(kind), extra_info(extra_info), related_token(related_token) {}
};

class ErrStream {
private:
  std::ostream &out;
  int tokenizer_errors;
  int syntax_errors;
  int lexical_errors;
  // and so on

public:
  ErrStream(std::ostream &out)
      : out(out), syntax_errors(0), lexical_errors(0) {}

  int TokenizerErrors() { return tokenizer_errors; }

  int SyntaxErrors() { return syntax_errors; }

  int LexicalErrors() { return lexical_errors; }

  /*
    Logs the error for when you have an undefined token;
    i.e. x = 1 ` y;
  */
  void ReportUndefinedToken(std::string token_data, LineRange pos);

  /*
    For when you expected a token but got EOF instead.
  */
  void ReportExpectedToken(Token::Kind expected, LineRange cur);

  /*
    For when you didn't expect the token that you got given.
    This is more for the case when you were expecting a very specific token.
    For example you were expecting a 'comma' but got a number instead.
  */
  void ReportUnexpectedToken(Token::Kind expected, Token invalid);

  /*
    For when an operator was invalid basically identical to above
    but for the case when you weren't expecting a specific token
    but rather one of many. i.e. for 1 ~ 2 we weren't expecting the `~`
    so we can error and we can't tell which token we were looking for.
  */
  void ReportInvalidToken(Token invalid);

  /*
    For when you try to narrow a tokens type for example to get an assignment
    operator, and it fails.
  */
  void ReportInvalidTokenCast(Token invalid, std::string msg);
};

}

#endif