#ifndef ERR_STREAM_HPP
#define ERR_STREAM_HPP

#include <iostream>
#include <string>

#include "token_stream.hpp"

namespace porc {

namespace err {
  void PipeOutput(std::ostream &out);

  enum ErrType {
    TokenErr,
    SyntaxErr,
    SemanticErr,
  };

  int TokenizerErrors();

  int SyntaxErrors();

  int SemanticErrors();

  /*
    Logs the error for when you have an undefined token;
    i.e. x = 1 ` y;
  */
  void ReportUndefinedToken(std::string token_data, LineRange pos);

  /*
    For when you expected a token but got EOF instead.
  */
  void ReportExpectedToken(Token::Kind expected, LineRange cur);

  /*
    For when you didn't expect the token that you got given.
    This is more for the case when you were expecting a very specific token.
    For example you were expecting a 'comma' but got a number instead.
  */
  void ReportUnexpectedToken(Token::Kind expected, Token invalid);

  /*
    For when you are missing a token like `;` at the end of a line.
    Gives a nicer error then saying the error is on the next line.
  */
  void ReportMissingToken(Token::Kind expected, LineRange pos);

  /*
    For when an operator was invalid basically identical to above
    but for the case when you weren't expecting a specific token
    but rather one of many. i.e. for 1 ~ 2 we weren't expecting the `~`
    so we can error and we can't tell which token we were looking for.
  */
  void ReportInvalidToken(Token invalid);

  /*
    For when you want to report a custom error that isn't really nicely grouped.
  */
  void ReportCustomErr(std::string msg, std::optional<LineRange> pos,
                       ErrType type, std::string carat_msg = "");

  /*
    For when you try to narrow a tokens type for example to get an assignment
    operator, and it fails.
  */
  void ReportInvalidTokenCast(Token invalid, std::string msg);

  /*
    For when you have a definition that is repeated and conflicts.
  */
  void ReportDualDefinition(std::string msg, LineRange first, LineRange second,
                            ErrType type, std::string carat_msg_first = "",
                            std::string carat_msg_second = "");

  /*
    Prints line information + some carat information if given.
  */
  void PrintLineData(LineRange pos, std::string carat_extra = "");
}

}

#endif