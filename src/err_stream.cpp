#include "err_stream.hpp"

#include <fstream>
#include <rang.hpp>

namespace porc {

namespace err {

std::mutex g_write_mutex;
std::ostream &out = std::cerr;
int tokenizer_errors = 0;
int syntax_errors = 0;
int semantic_errors = 0;

int TokenizerErrors() { return tokenizer_errors; }

int SyntaxErrors() { return syntax_errors; }

int SemanticErrors() { return semantic_errors; }

void IncrementErr(ErrType type) {
  switch (type) {
    case TokenErr:     tokenizer_errors++;   break;
    case SyntaxErr:    syntax_errors++;      break;
    case SemanticErr:  semantic_errors++;    break;
  }
}

void PrintFileLine(LineRange pos, std::string carat_extra = "") {
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

void PrintLineData(LineRange pos, std::string carat_extra) {
  // so we don't double lock ourselves
  // @BAD: this is just kinda bad, either we don't want to expose this
  //       or we should place an atomic around the err stream object itself
  std::scoped_lock g(g_write_mutex);

  PrintFileLine(pos, carat_extra);
}

void ReportUndefinedToken(std::string token_data, LineRange pos) {
  std::scoped_lock g(g_write_mutex);

  out << rang::fg::red << "Error " << pos.file_name << "(" << pos
                          << "): Can't form a token from; "
                          << token_data << rang::style::reset << std::endl;
  // @TODO: implement a lookup to see possible tokens
  //        i.e. if they write `+~` we could say did you mean `+`, `+=`
  IncrementErr(TokenErr);
}

void ReportExpectedToken(Token::Kind expected, LineRange cur) {
  std::scoped_lock g(g_write_mutex);

  out << rang::fg::red << "Error " << cur.file_name << "(" << cur
                          << "): was expecting '"
                          << Token::GetKindErrorMsg(expected)
                          << "'" << rang::style::reset << std::endl;
  PrintFileLine(cur);
  IncrementErr(SyntaxErr);
}

void ReportCustomErr(std::string msg, std::optional<LineRange> pos,
                                ErrType type, std::string carat_msg) {
  std::scoped_lock g(g_write_mutex);

  if (pos) {
    out << rang::fg::red << "Error " << pos->file_name << "(" << *pos
        << "): " << msg
        << rang::style::reset << std::endl;
    PrintFileLine(*pos, carat_msg);
  }
  else {
    out << rang::fg::red << "Error: " << msg << rang::style::reset << std::endl;
  }
  IncrementErr(type);
}

void ReportMissingToken(Token::Kind expected, LineRange pos) {
  std::scoped_lock g(g_write_mutex);

  out << rang::fg::red << "Error " << pos.file_name << "("
                                  << pos << "): Missing '" 
                                    << Token::GetKindErrorMsg(expected)
                                  << "'"
                                  << rang::style::reset << std::endl;
  PrintFileLine(pos, Token::GetKindErrorMsg(expected));
  IncrementErr(SyntaxErr);
}

void ReportUnexpectedToken(Token::Kind expected, Token invalid) {
  std::scoped_lock g(g_write_mutex);

  out << rang::fg::red << "Error " << invalid.pos.file_name << "("
                                  << invalid.pos << "): was expecting '"
                                    << Token::GetKindErrorMsg(expected)
                                  << "' but instead got '"
                                    << invalid.ToErrorMsg()
                                  << "'"
                                  << rang::style::reset << std::endl;
  PrintFileLine(invalid.pos, Token::GetKindErrorMsg(expected));
  IncrementErr(SyntaxErr);
}

void ReportInvalidToken(Token invalid) {
  std::scoped_lock g(g_write_mutex);

  // @TODO: implement some nicer information this is very bare
  out << rang::fg::red << "Error " << invalid.pos.file_name << "("
                                  << invalid.pos << "): Invalid token '"
                                  << invalid.ToErrorMsg() << "'"
                                  << rang::style::reset << std::endl;
  PrintFileLine(invalid.pos);
  IncrementErr(SyntaxErr);
}

void ReportInvalidTokenCast(Token invalid, std::string msg) {
  std::scoped_lock g(g_write_mutex);

  out << rang::fg::red << "Error " << invalid.pos.file_name << "("
                                  << invalid.pos << "): Invalid token '"
                                  << invalid.ToErrorMsg() << "'"
                                  << rang::style::reset << std::endl;
  PrintFileLine(invalid.pos, msg);
  IncrementErr(SyntaxErr);
}

void ReportDualDefinition(std::string msg, LineRange first,
    LineRange second, ErrType type, std::string carat_msg_first,
    std::string carat_msg_second) {
  std::scoped_lock g(g_write_mutex);

  out << rang::fg::red << "Error " << second.file_name << "(" << second << "):"
                       << "Conflicting definition with definition; "
                       << first.file_name << "(" << first << ")"
                       << "\n" << msg << rang::style::reset << std::endl;
  out << "\nThe second definition:" << std::endl;
  PrintFileLine(second, carat_msg_second);
  PrintFileLine(second, carat_msg_second);
  IncrementErr(type);
}

}
}