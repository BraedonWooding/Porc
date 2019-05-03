#include "err_stream.hpp"

namespace porc::internals {

void ErrStream::ReportUndefinedToken(std::string token_data, LineRange pos) {
  out << "Error " << pos.file_name << "(" << pos
                          << "): Can't form a token from; "
                          << token_data << std::endl;
  // @TODO: implement a lookup to see possible tokens
  //        i.e. if they write `+~` we could say did you mean `+`, `+=`
  tokenizer_errors++;
}

void ErrStream::ReportExpectedToken(Token::Kind expected, LineRange cur) {
  out << "Error " << cur.file_name << "(" << cur
                          << "): Wasn't expecting EOF, was expecting to get; '"
                          << Token::GetKindErrorMsg(expected)
                          << "'" << std::endl;
  syntax_errors++;
}

void ErrStream::ReportCustomErr(std::string msg, std::optional<LineRange> pos,
                                ErrType type) {
  switch (type) {
    case ErrStream::TokenErr:     tokenizer_errors++;   break;
    case ErrStream::SyntaxErr:    syntax_errors++;  break;
    case ErrStream::LexicalErr:   lexical_errors++;     break;
    case ErrStream::SemanticErr:  semantic_errors++;    break;
  }
  if (pos) out << "Error " << pos->file_name << "(" << *pos << "): " << msg
           << std::endl;
  else     out << "Error: " << msg << std::endl;
}

void ErrStream::ReportUnexpectedToken(Token::Kind expected, Token invalid) {
  out << "Error " << invalid.pos.file_name << "(" << invalid.pos
                                  << "): Wasn't expecting '"
                                    << invalid.ToErrorMsg()
                                  << "' was expecting; '" 
                                    << Token::GetKindErrorMsg(expected)
                                  << "'"
                                  << std::endl;
  syntax_errors++;
}

void ErrStream::ReportInvalidToken(Token invalid) {
  // @TODO: implement some nicer information this is very bare
  out << "Error " << invalid.pos.file_name << "(" << invalid.pos
                                  << "): Invalid token '"
                                  << invalid.ToErrorMsg() << "'" << std::endl;
  syntax_errors++;
}

void ErrStream::ReportInvalidTokenCast(Token invalid, std::string msg) {
  ReportInvalidToken(invalid);
  out << "Valid Tokens; " << msg << std::endl;
  // @NOTE: Intentionally not incrementing syntax errors
  //        since the report will do that.  Probably needs some
  //        @CLEANUP eventually.
}

}