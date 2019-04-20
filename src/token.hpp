#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <iostream>
#include <variant>
#include <string>
#include <optional>
#include "printer_helpers.hpp"
#include "reader.hpp"
#include "defs.hpp"

// @TEST: test if this generator is faster/slower than just loading all tokens at once
//        - make sure to test using parallelisation as well since that could be much slower.

namespace porc::internals {

class Token {
 public:
  enum Kind {
#include "token_list.inc"
    NumTokens,
  };
  // @TODO: @FIXME: @WHY: this needs to support unicode
  // probably a mixture of utf-32/utf-16 encoding to be fast
  // ditch utf-8 cause its shit.  The problem is with `char`
  // and kinda with std::string I guess??
  // Also we should support much larger integers and floating points
  std::variant<std::string, double, std::int64_t, char> data;

  Token::Kind type;
  LineRange pos;

  static const char *GetKindErrorMsg(Token::Kind type);

  const char *ToName() const;
  std::string ToString() const;
  const char *ToErrorMsg() const;

  Token()
      : pos(-1, -1, -1, -1, 
            "Internal Compiler Error: This token shouldn't be outputted"),
        type(Token::Undefined) { }

  Token(Token::Kind type, LineRange pos) : pos(pos), type(type) { }

  template<typename T>
  Token(Token::Kind type, LineRange pos, T data) : pos(pos), type(type), data(data) { }

  operator bool() const;

  bool IsAssignmentOp() const;
  bool IsPrefixOp() const;
  bool IsMultiplicativeOp() const;
  bool IsAdditiveOp() const;
  bool IsRelationalOp() const;
  bool IsEqualityOp() const;
};

}

#endif