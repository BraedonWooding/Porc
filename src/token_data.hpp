/* Auto Generated File */
#ifndef TOKEN_DATA_HPP
#define TOKEN_DATA_HPP

#include "token.hpp"

namespace porc {
  const char *tokenToStr(Token::Kind token);
  const char *tokenToName(Token::Kind token);
  Token::Kind tokenFromStr(std::string_view str);
}

#endif
