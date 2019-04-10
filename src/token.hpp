#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <iostream>
#include <variant>
#include <string>
#include <optional>
#include "printer_helpers.hpp"
#include "reader.hpp"
#include "defs.hpp"

namespace porc::internals {

class Token {
 public:
  enum class Kind {
#include "token_list.inc"
    NumTokens,
  };
  // @TODO: @FIXME: @WHY: this needs to support unicode
  // probably a mixture of utf-32/utf-16 encoding to be fast
  // ditch utf-8 cause its shit.
  std::variant<std::string, double, std::int64_t, char> data;

  Token::Kind type;
  LineRange pos;

  const char *ToName() const;
  std::string ToString() const;
  const char *ToErrorMsg() const;

  Token() : pos(LineRange::Null()) {};
  Token(Token::Kind type, LineRange pos) : pos(pos), type(type) { }

  template<typename T>
  Token(Token::Kind type, LineRange pos, T data) : pos(pos), type(type), data(data) { }

  operator bool() const;

  bool IsAssignmentOp() const;
  bool IsPostfixOp() const;
  bool IsPrefixOp() const;
  bool IsMultiplicativeOp() const;
  bool IsAdditiveOp() const;
  bool IsRelationalOp() const;
  bool IsEqualityOp() const;

#include "token_constants_definitions.inc"
};

#include "token_constants.inc"

}

#endif