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

/*
  @Question: does this leak memory?  Because std::string's destructor isn't
             virtual this could leak, but only if the allocations are well
             'leakable' in this case they are since LineRange contains a string
             HOWEVER this only occurs for things where you allocate a line str
             then pass it as a std::string, which we don't do.
  @CLEANUP:  So I guess we need to come up with a better solution??
             I just don't want to have a whole class for just adding an extra
             member field, composition would require a lot of utility code.
*/
class LineStr : public std::string {
 public:
  LineRange pos;

  LineStr(LineRange pos, std::string wrap)
      : std::string(std::move(wrap)), pos(pos) { }
};

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
  std::string ToErrorMsg() const;

  Token()
      : pos(-1, -1, -1, -1, 
            "Internal Compiler Error: This token shouldn't be outputted"),
        type(Token::Undefined) { }

  Token(Token::Kind type, LineRange pos) : pos(pos), type(type) { }

  template<typename T>
  Token(Token::Kind type, LineRange pos, T data) : pos(pos), type(type), data(data) { }

  std::optional<LineStr> ToLineStr() const {
    if (auto str = std::get_if<std::string>(&data)) {
      return LineStr(pos, *str);
    } else {
      return std::nullopt;
    }
  }

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