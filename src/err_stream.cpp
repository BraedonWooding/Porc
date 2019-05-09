#include "err_stream.hpp"

#include <fstream>
#include <rang.hpp>

namespace porc::internals {

void ErrStream::PrintFileLine(LineRange pos, std::string carat_extra) {
    std::ifstream file(pos.file_name);
    std::string line;

    unsigned int line_number = 0;
    auto line_str = std::to_string(pos.line_end);
    out << std::string(line_str.size() + 1, ' ') << '|' << std::endl;
    while (std::getline(file, line) && line_number < pos.line_end)
    {
      line_number++;
      // @HACK: (Slight) just to give nicer err messages
      //        we are going to give err messages minus one line
      //        this isn't a perfect fix and we should rather kind of look
      //        and concatenating blank lines so we don't just give more blank
      //        lines
      // @NOTE: using ::isspace so its clear its not the std one
      if (line_number >= pos.line_start - 1 && line_number <= pos.line_end &&
          !std::all_of(line.cbegin(), line.cend(), ::isspace))
      {
        line_str = std::to_string(line_number);
        out << line_str << " |" << "    " << line << std::endl;
        if (line_number == pos.line_start) {
          out << std::string(line_str.size() + 1, ' ') << '|'
              << std::string(4 + pos.col_start - 1, ' ')
              << rang::fg::green
              << std::string(pos.col_end - pos.col_start + 1, '^')
              << rang::fg::blue << " " << carat_extra << rang::style::reset
              << std::endl;
        }
      }
    }
    out << std::string(line_str.size() + 1, ' ') << '|' << std::endl;
    if (line_number != pos.line_end) {
      out << "Internal Compiler Error Invalid Position: " << pos << std::endl;
    }
}

void ErrStream::ReportUndefinedToken(std::string token_data, LineRange pos) {
  out << rang::fg::red << "Error " << pos.file_name << "(" << pos
                          << "): Can't form a token from; "
                          << token_data << rang::style::reset << std::endl;
  // @TODO: implement a lookup to see possible tokens
  //        i.e. if they write `+~` we could say did you mean `+`, `+=`
  tokenizer_errors++;
}

void ErrStream::ReportExpectedToken(Token::Kind expected, LineRange cur) {
  out << rang::fg::red << "Error " << cur.file_name << "(" << cur
                          << "): was expecting '"
                          << Token::GetKindErrorMsg(expected)
                          << "'" << rang::style::reset << std::endl;
  PrintFileLine(cur);
  syntax_errors++;
}

void ErrStream::ReportCustomErr(std::string msg, std::optional<LineRange> pos,
                                ErrType type, std::string carat_msg) {
  switch (type) {
    case ErrStream::TokenErr:     tokenizer_errors++;   break;
    case ErrStream::SyntaxErr:    syntax_errors++;      break;
    case ErrStream::SemanticErr:  semantic_errors++;    break;
  }
  if (pos) {
    out << rang::fg::red << "Error " << pos->file_name << "(" << *pos
        << "): " << msg
        << rang::style::reset << std::endl;
    PrintFileLine(*pos, carat_msg);
  }
  else {
    out << rang::fg::red << "Error: " << msg << rang::style::reset << std::endl;
  }
}

void ErrStream::ReportMissingToken(Token::Kind expected, LineRange pos) {
  out << rang::fg::red << "Error " << pos.file_name << "("
                                  << pos << "): Missing '" 
                                    << Token::GetKindErrorMsg(expected)
                                  << "'"
                                  << rang::style::reset << std::endl;
  PrintFileLine(pos, Token::GetKindErrorMsg(expected));
  syntax_errors++;
}

void ErrStream::ReportUnexpectedToken(Token::Kind expected, Token invalid) {
  out << rang::fg::red << "Error " << invalid.pos.file_name << "("
                                  << invalid.pos << "): was expecting '"
                                    << Token::GetKindErrorMsg(expected)
                                  << "' but instead got '"
                                    << invalid.ToErrorMsg()
                                  << "'"
                                  << rang::style::reset << std::endl;
  PrintFileLine(invalid.pos, Token::GetKindErrorMsg(expected));
  syntax_errors++;
}

void ErrStream::ReportInvalidToken(Token invalid) {
  // @TODO: implement some nicer information this is very bare
  out << rang::fg::red << "Error " << invalid.pos.file_name << "("
                                  << invalid.pos << "): Invalid token '"
                                  << invalid.ToErrorMsg() << "'"
                                  << rang::style::reset << std::endl;
  PrintFileLine(invalid.pos);
  syntax_errors++;
}

void ErrStream::ReportInvalidTokenCast(Token invalid, std::string msg) {
  out << rang::fg::red << "Error " << invalid.pos.file_name << "("
                                  << invalid.pos << "): Invalid token '"
                                  << invalid.ToErrorMsg() << "'"
                                  << rang::style::reset << std::endl;
  PrintFileLine(invalid.pos, msg);
  syntax_errors++;
}

}