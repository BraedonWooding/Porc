/* Auto Generated File */
#include "token_data.hpp"

namespace porc {

const char *tokenToStr(Token::Kind token) {
  switch (token) {
    case Token::Kind::Comma: return ",";
    case Token::Kind::SemiColon: return ";";
    case Token::Kind::LeftParen: return "(";
    case Token::Kind::RightParen: return ")";
    case Token::Kind::LeftBrace: return "{";
    case Token::Kind::RightBrace: return "}";
    case Token::Kind::LeftBracket: return "[";
    case Token::Kind::RightBracket: return "]";
    case Token::Kind::LineComment: return "#";
    case Token::Kind::BlockComment: return "/*";
    case Token::Kind::LessThan: return "<";
    case Token::Kind::GreaterThan: return ">";
    case Token::Kind::Equal: return "==";
    case Token::Kind::NotEqual: return "!=";
    case Token::Kind::LessThanEqual: return "<=";
    case Token::Kind::GreaterThanEqual: return ">=";
    case Token::Kind::Negate: return "!";
    case Token::Kind::And: return "&&";
    case Token::Kind::Or: return "||";
    case Token::Kind::Add: return "+";
    case Token::Kind::Subtract: return "-";
    case Token::Kind::Divide: return "/";
    case Token::Kind::Multiply: return "*";
    case Token::Kind::Power: return "**";
    case Token::Kind::Modulus: return "%";
    case Token::Kind::IntegerDivide: return "//";
    case Token::Kind::Assign: return "=";
    case Token::Kind::AddAssign: return "+=";
    case Token::Kind::SubtractAssign: return "-=";
    case Token::Kind::MultiplyAssign: return "*=";
    case Token::Kind::DivideAssign: return "/=";
    case Token::Kind::PowerAssign: return "**=";
    case Token::Kind::IntegerDivideAssign: return "%/=";
    case Token::Kind::ModulusAssign: return "%=";
    case Token::Kind::FatArrow: return "=>";
    case Token::Kind::FoldLeft: return "<|";
    case Token::Kind::FoldRight: return "|>";
    case Token::Kind::ReturnType: return "->";
    case Token::Kind::Colon: return ":";
    case Token::Kind::DoubleColon: return "::";
    case Token::Kind::ColonAssign: return ":=";
    case Token::Kind::Variant: return "|";
    case Token::Kind::Dot: return ".";
    case Token::Kind::Range: return "..";
    case Token::Kind::Macro: return "@";
    case Token::Kind::Generic: return "$";
    case Token::Kind::True: return "true";
    case Token::Kind::False: return "false";
    case Token::Kind::Void: return "void";
    case Token::Kind::Type: return "type";
    case Token::Kind::Yield: return "yield";
    case Token::Kind::Let: return "let";
    case Token::Kind::Return: return "return";
    case Token::Kind::While: return "while";
    case Token::Kind::For: return "for";
    case Token::Kind::Break: return "break";
    case Token::Kind::Continue: return "continue";
    case Token::Kind::In: return "in";
    case Token::Kind::If: return "if";
    case Token::Kind::Is: return "is";
    case Token::Kind::Else: return "else";
    default: return nullptr;
  }
}

const char *tokenToName(Token::Kind token) {
  switch (token) {    case Token::Kind::Undefined: return "Undefined";
    case Token::Kind::Continuer: return "Continuer";
    case Token::Kind::Identifier: return "Identifier";
    case Token::Kind::Str: return "Str";
    case Token::Kind::Flt: return "Flt";
    case Token::Kind::Int: return "Int";
    case Token::Kind::Char: return "Char";
    case Token::Kind::EndOfFile: return "EndOfFile";
    case Token::Kind::Comma: return "Comma";
    case Token::Kind::SemiColon: return "SemiColon";
    case Token::Kind::LeftParen: return "LeftParen";
    case Token::Kind::RightParen: return "RightParen";
    case Token::Kind::LeftBrace: return "LeftBrace";
    case Token::Kind::RightBrace: return "RightBrace";
    case Token::Kind::LeftBracket: return "LeftBracket";
    case Token::Kind::RightBracket: return "RightBracket";
    case Token::Kind::LineComment: return "LineComment";
    case Token::Kind::BlockComment: return "BlockComment";
    case Token::Kind::LessThan: return "LessThan";
    case Token::Kind::GreaterThan: return "GreaterThan";
    case Token::Kind::Equal: return "Equal";
    case Token::Kind::NotEqual: return "NotEqual";
    case Token::Kind::LessThanEqual: return "LessThanEqual";
    case Token::Kind::GreaterThanEqual: return "GreaterThanEqual";
    case Token::Kind::Negate: return "Negate";
    case Token::Kind::And: return "And";
    case Token::Kind::Or: return "Or";
    case Token::Kind::Add: return "Add";
    case Token::Kind::Subtract: return "Subtract";
    case Token::Kind::Divide: return "Divide";
    case Token::Kind::Multiply: return "Multiply";
    case Token::Kind::Power: return "Power";
    case Token::Kind::Modulus: return "Modulus";
    case Token::Kind::IntegerDivide: return "IntegerDivide";
    case Token::Kind::Assign: return "Assign";
    case Token::Kind::AddAssign: return "AddAssign";
    case Token::Kind::SubtractAssign: return "SubtractAssign";
    case Token::Kind::MultiplyAssign: return "MultiplyAssign";
    case Token::Kind::DivideAssign: return "DivideAssign";
    case Token::Kind::PowerAssign: return "PowerAssign";
    case Token::Kind::IntegerDivideAssign: return "IntegerDivideAssign";
    case Token::Kind::ModulusAssign: return "ModulusAssign";
    case Token::Kind::FatArrow: return "FatArrow";
    case Token::Kind::FoldLeft: return "FoldLeft";
    case Token::Kind::FoldRight: return "FoldRight";
    case Token::Kind::ReturnType: return "ReturnType";
    case Token::Kind::Colon: return "Colon";
    case Token::Kind::DoubleColon: return "DoubleColon";
    case Token::Kind::ColonAssign: return "ColonAssign";
    case Token::Kind::Variant: return "Variant";
    case Token::Kind::Dot: return "Dot";
    case Token::Kind::Range: return "Range";
    case Token::Kind::Macro: return "Macro";
    case Token::Kind::Generic: return "Generic";
    case Token::Kind::True: return "True";
    case Token::Kind::False: return "False";
    case Token::Kind::Void: return "Void";
    case Token::Kind::Type: return "Type";
    case Token::Kind::Yield: return "Yield";
    case Token::Kind::Let: return "Let";
    case Token::Kind::Return: return "Return";
    case Token::Kind::While: return "While";
    case Token::Kind::For: return "For";
    case Token::Kind::Break: return "Break";
    case Token::Kind::Continue: return "Continue";
    case Token::Kind::In: return "In";
    case Token::Kind::If: return "If";
    case Token::Kind::Is: return "Is";
    case Token::Kind::Else: return "Else";
    default: return nullptr;
  }
}

Token::Kind tokenFromStr(std::string_view str) {
  if (str == ",") return Token::Kind::Comma;
  if (str == ";") return Token::Kind::SemiColon;
  if (str == "(") return Token::Kind::LeftParen;
  if (str == ")") return Token::Kind::RightParen;
  if (str == "{") return Token::Kind::LeftBrace;
  if (str == "}") return Token::Kind::RightBrace;
  if (str == "[") return Token::Kind::LeftBracket;
  if (str == "]") return Token::Kind::RightBracket;
  if (str == "#") return Token::Kind::LineComment;
  if (str == "/*") return Token::Kind::BlockComment;
  if (str == "<") return Token::Kind::LessThan;
  if (str == ">") return Token::Kind::GreaterThan;
  if (str == "==") return Token::Kind::Equal;
  if (str == "!=") return Token::Kind::NotEqual;
  if (str == "<=") return Token::Kind::LessThanEqual;
  if (str == ">=") return Token::Kind::GreaterThanEqual;
  if (str == "!") return Token::Kind::Negate;
  if (str == "&&") return Token::Kind::And;
  if (str == "||") return Token::Kind::Or;
  if (str == "+") return Token::Kind::Add;
  if (str == "-") return Token::Kind::Subtract;
  if (str == "/") return Token::Kind::Divide;
  if (str == "*") return Token::Kind::Multiply;
  if (str == "**") return Token::Kind::Power;
  if (str == "%") return Token::Kind::Modulus;
  if (str == "//") return Token::Kind::IntegerDivide;
  if (str == "=") return Token::Kind::Assign;
  if (str == "+=") return Token::Kind::AddAssign;
  if (str == "-=") return Token::Kind::SubtractAssign;
  if (str == "*=") return Token::Kind::MultiplyAssign;
  if (str == "/=") return Token::Kind::DivideAssign;
  if (str == "**=") return Token::Kind::PowerAssign;
  if (str == "%/=") return Token::Kind::IntegerDivideAssign;
  if (str == "%=") return Token::Kind::ModulusAssign;
  if (str == "=>") return Token::Kind::FatArrow;
  if (str == "<|") return Token::Kind::FoldLeft;
  if (str == "|>") return Token::Kind::FoldRight;
  if (str == "->") return Token::Kind::ReturnType;
  if (str == ":") return Token::Kind::Colon;
  if (str == "::") return Token::Kind::DoubleColon;
  if (str == ":=") return Token::Kind::ColonAssign;
  if (str == "|") return Token::Kind::Variant;
  if (str == ".") return Token::Kind::Dot;
  if (str == "..") return Token::Kind::Range;
  if (str == "@") return Token::Kind::Macro;
  if (str == "$") return Token::Kind::Generic;
  if (str == "true") return Token::Kind::True;
  if (str == "false") return Token::Kind::False;
  if (str == "void") return Token::Kind::Void;
  if (str == "type") return Token::Kind::Type;
  if (str == "yield") return Token::Kind::Yield;
  if (str == "let") return Token::Kind::Let;
  if (str == "return") return Token::Kind::Return;
  if (str == "while") return Token::Kind::While;
  if (str == "for") return Token::Kind::For;
  if (str == "break") return Token::Kind::Break;
  if (str == "continue") return Token::Kind::Continue;
  if (str == "in") return Token::Kind::In;
  if (str == "if") return Token::Kind::If;
  if (str == "is") return Token::Kind::Is;
  if (str == "else") return Token::Kind::Else;
  return Token::Kind::Undefined;
}
}
