#include "token.hpp"

#include "token_data.hpp"

namespace porc::internals {

Token::operator bool() const {
  return type != Token::Kind::Undefined && type != Token::Kind::EndOfFile;
}

std::string Token::ToString() const {
  if (type == Token::Kind::Str || type == Token::Kind::Identifier)
    return std::get<std::string>(data);
  char *out = NULL;
  if (type == Token::Kind::LineComment) {
    return std::string("//").append(std::get<std::string>(data));
  }
  if (type == Token::Kind::BlockComment) {
    return std::string("/*").append(std::get<std::string>(data)).append("*/");
  }
  if (type == Token::Kind::Flt) return std::to_string(std::get<double>(data));
  if (type == Token::Kind::Int) return std::to_string(std::get<i64>(data));
  if (out != NULL) return out;
  int type = static_cast<int>(type);
  if (tokenToStrMap[type] != NULL) return std::string(tokenToStrMap[type]);

  Unreachable("Case not handled");
}

const char *Token::ToErrorMsg() const {
  int type = static_cast<int>(type);
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
    case Token::Kind::AddAssign:
    case Token::Kind::SubtractAssign:
    case Token::Kind::Equal:
    case Token::Kind::DivideAssign:
    case Token::Kind::PowerAssign:
    case Token::Kind::ModulusAssign:
    case Token::Kind::MultiplyAssign:
    case Token::Kind::IntegerDivideAssign:
      return true;
    default:
      return false;
  }
}

bool Token::IsPostfixOp() const {
  switch (type) {
    case Token::Kind::Increment:
    case Token::Kind::Decrement:
      return true;
    default:
      return false;
  }
}

bool Token::IsPrefixOp() const {
  switch (type) {
    case Token::Kind::Increment:
    case Token::Kind::Decrement:
    case Token::Kind::Negate:
    case Token::Kind::Subtract:
    case Token::Kind::Add:
      return true;
    default:
      return false;
  }
}

bool Token::IsMultiplicativeOp() const {
  switch (type) {
    case Token::Kind::Multiply:
    case Token::Kind::Divide:
    case Token::Kind::Modulus:
    case Token::Kind::IntegerDivide:
      return true;
    default:
      return false;
  }
}

bool Token::IsAdditiveOp() const {
  switch (type) {
    case Token::Kind::Add:
    case Token::Kind::Subtract:
      return true;
    default:
      return false;
  }
}

bool Token::IsRelationalOp() const {
  switch (type) {
    case Token::Kind::GreaterThan:
    case Token::Kind::GreaterThanEqual:
    case Token::Kind::LessThan:
    case Token::Kind::LessThanEqual:
      return true;
    default:
      return false;
  }
}

bool Token::IsEqualityOp() const {
  switch (type) {
    case Token::Kind::Equal:
    case Token::Kind::NotEqual:
      return true;
    default:
      return false;
  }
}
}
