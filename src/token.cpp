#include "token.hpp"

#include "token_data.hpp"

namespace porc::internals {

Token::operator bool() const {
  return type != Token::Undefined && type != Token::EndOfFile;
}

std::string Token::ToString() const {
  if (type == Token::Str || type == Token::Identifier)
    return std::get<std::string>(data);
  char *out = NULL;
  if (type == Token::LineComment) {
    return std::string("//").append(std::get<std::string>(data));
  }
  if (type == Token::BlockComment) {
    return std::string("/*").append(std::get<std::string>(data)).append("*/");
  }
  if (type == Token::Flt) return std::to_string(std::get<double>(data));
  if (type == Token::Int) return std::to_string(std::get<i64>(data));
  if (out != NULL) return out;
  int type = static_cast<int>(type);
  if (tokenToStrMap[type] != NULL) return std::string(tokenToStrMap[type]);

  Unreachable("Case not handled");
}

const char *Token::ToErrorMsg() const {
  return GetKindErrorMsg(type);
}

const char *Token::GetKindErrorMsg(Token::Kind kind) {
  int type = static_cast<int>(kind);
  if (tokenToStrMap[type] != NULL) return tokenToStrMap[type];
  if (tokenToNameMap[type] != NULL) return tokenToNameMap[type];
  Unreachable("Case not handled");
}

const char *Token::ToName() const {
  return tokenToNameMap[static_cast<int>(type)];
  Unreachable("Case not handled");
}

bool Token::IsAssignmentOp() const {
  switch (type) {
    case Token::AddAssign:
    case Token::SubtractAssign:
    case Token::Equal:
    case Token::DivideAssign:
    case Token::PowerAssign:
    case Token::ModulusAssign:
    case Token::MultiplyAssign:
    case Token::IntegerDivideAssign:
      return true;
    default:
      return false;
  }
}

bool Token::IsPrefixOp() const {
  switch (type) {
    case Token::Negate:
    case Token::Subtract:
    case Token::Add:
      return true;
    default:
      return false;
  }
}

bool Token::IsMultiplicativeOp() const {
  switch (type) {
    case Token::Multiply:
    case Token::Divide:
    case Token::Modulus:
    case Token::IntegerDivide:
      return true;
    default:
      return false;
  }
}

bool Token::IsAdditiveOp() const {
  switch (type) {
    case Token::Add:
    case Token::Subtract:
      return true;
    default:
      return false;
  }
}

bool Token::IsRelationalOp() const {
  switch (type) {
    case Token::GreaterThan:
    case Token::GreaterThanEqual:
    case Token::LessThan:
    case Token::LessThanEqual:
      return true;
    default:
      return false;
  }
}

bool Token::IsEqualityOp() const {
  switch (type) {
    case Token::Equal:
    case Token::NotEqual:
      return true;
    default:
      return false;
  }
}
}