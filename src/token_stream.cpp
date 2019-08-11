#include "token_stream.hpp"

#include <locale>
#include <optional>
#include <string>
#include "helper.hpp"
#include "token_data.hpp"

/*
  Helpful notes:
  This is mostly here since our nature of reading is a little odd perhaps?
  And so I don't forget what I did.

  - We have boundary reads which means that since we read in segments
    (rather than whole file into a string) we need to take care of the rare
    cases where a token is spread over one or multiple boundaries
    - In actual practice this will only effect large strings/comments
      since the chance it will hit a token is pretty small most likely it'll hit
      whitespace or an identifier which are both really cheap in terms of how
      we cope with it
      - They are only 'expensive' when we have to keep parts of the
        current buffer and can't ReadAll (still it is quite cheap)
  - We have to be wary of the boundary reads i.e. ['\']['\'] has to be passed
  correctly
    - A few helpful functions; Read/ReadAll
  - We also have to adjust col/line appropriately but that is pretty easy
    and mostly covered by the parse functions.
  - We do it this way since it means a few things;
    - More efficient/Usable on memory sparce systems (low amount of ram)
    - Better IO pipelining (consistently small reads is better than
      one large read).
    - Supports insanely large files better
    - Doesn't cause huge segmentation (this really only effects interpreters
  which we are)
      - This is because if we allocate a huge memory block before we are even
  running the program then deallocating before we run it we could possibly cause
  some segmentation (of course in reality large blocks of segmentation are
  rarely an issue)

  @NOTE:  make sure you are using fadvise (FADV_NOREUSE | FADV_SEQUENTIAL)
      to get the most performance out of this way of reading the files.

  @TODO:  decouple these functions
*/

namespace porc {

inline void TokenStream::BeginLineRange() {
  old_line = line;
  old_col = col;
}

// @TODO: the line ranges for this are slightly wrong since it'll represent
//        them as 0:-1 -> 1:-1 if col_offset < 0 and col and/or old_col = -1
//        maybe we should somehow handle this nicer.
inline LineRange TokenStream::EndLineRange(int col_offset) const {
  return LineRange(old_line, line, old_col, col + col_offset, GetFileName());
}

Token TokenStream::PeekCur() {
  if (cur_token_size == 0) Next();
  return cur;
}

Token TokenStream::PopCur() {
  if (cur_token_size == 0) Next();
  cur_token_size--;
  if (cur.type != Token::EndOfFile && cur.type != Token::Undefined) last = cur;
  Token tmp = cur;
  cur = next_cur;
  next_cur = Token();
  return tmp;
}

Token TokenStream::LastPopped() {
  Assert(last.type != Token::Undefined, "No last seen token");
  return last;
}

void TokenStream::Push(Token tok) {
  Assert(cur_token_size < TokenizerMaxLookahead,
         "Can't `Push()` consecutively more than `TokenizerMaxLookahead`",
         TokenizerMaxLookahead);
  next_cur = cur;
  cur = tok;
  cur_token_size++;
}

void TokenStream::Next() {
  Assert(cur_token_size < TokenizerMaxLookahead,
         "Can't `Next()` consecutively more than `Lookaheads`",
         TokenizerMaxLookahead);
  Token tok = Parse();
  if (ignore_comments) {
    while (tok.type == Token::LineComment || tok.type == Token::BlockComment) {
      tok = Parse();
    }
  }

  Push(tok);
}

void TokenStream::SkipWs() {
  Assert(IsEOF() || cur_index < read_size, "Precondition failed",
         "cur_index: ", cur_index, ", read_size: ", read_size);

  while (!IsEOF() && std::isspace(read_buf[cur_index])) {
    if (read_buf[cur_index] == '\n') {
      line++;
      col = 1;
    } else {
      col++;
    }
    cur_index++;
    if (cur_index == read_size) Read();
  }

  Assert(cur_index < read_size || IsEOF(), "Postcondition failed",
         "cur_index: ", cur_index, read_size, ", read_size: ", read_size);
}

std::string TokenStream::ParseLineComment() {
  std::string buf;
  while (true) {
    if (cur_index == read_size) Read();
    if (IsEOF() || read_buf[cur_index] == '\n') break;
    buf.push_back(read_buf[cur_index++]);
    col++;
  }
  if (read_buf[cur_index] == '\n') {
    col = 1;
    line++;
    cur_index++;
  }

  return buf;
}

// supports nesting
std::optional<std::string> TokenStream::ParseBlockComment() {
  std::string buf;
  int depth = 1;

  while (depth > 0) {
    if (cur_index == read_size) Read();
    // block comment not ended
    if (IsEOF()) return std::nullopt;
    if (read_buf[cur_index] == '*') {
      // since we have read all already at the top
      // we are sure that cur_index must be < read_size
      // else its an EOF
      cur_index++;
      col++;
      if (cur_index == read_size) Read();
      if (IsEOF()) return std::nullopt;

      if (read_buf[cur_index] == '/') {
        depth--;
        col++;
      } else {
        buf.push_back(read_buf[cur_index]);
        if (read_buf[cur_index] == '\n') {
          col = 1;
          line++;
        }
      }
      cur_index++;
    } else if (read_buf[cur_index] == '/') {
      if (read_buf[cur_index] == '*') {
        depth++;
        col++;
      } else {
        buf.push_back(read_buf[cur_index]);
        if (read_buf[cur_index] == '\n') {
          col = 1;
          line++;
        }
      }
      cur_index++;
    } else {
      buf.push_back(read_buf[cur_index]);
      cur_index++;
      if (read_buf[cur_index] == '\n') {
        col = 1;
        line++;
      }
    }
  }
  return buf;
}

Token TokenStream::ParseSimpleToken() {
  if (cur_index > 0) ReadOffset();
  Token cur;
  cur.type = Token::Undefined;

  BeginLineRange();

  size_t i = 1;
  std::string_view view = read_buf;

  if (std::isalpha(read_buf[cur_index])) {
    // read till non letter
    // it is <= read_size since the + i represents an offset length
    while (std::isalpha(read_buf[cur_index + i]) && cur_index + i <= read_size)
      i++;
    cur.type = tokenFromStr(view.substr(cur_index, i));
    if (cur.type != Token::Undefined) {
      cur_index += i;
    }
  } else {
    Token::Kind prev = Token::Undefined;
    Token::Kind kind = tokenFromStr(view.substr(cur_index, i));
    while (cur_index + i <= read_size &&
           (kind != Token::Undefined || prev == Token::Undefined)) {
      prev = kind;
      i++;
      kind = tokenFromStr(view.substr(cur_index, i));
    }

    if (prev != Token::Undefined) {
      cur.type = prev;
      cur_index += i - 1;
    }
  }

  cur.pos = EndLineRange();

  Assert(IsEOF() || cur_index <= read_size, "Postcondition Failed",
         "read_size: ", read_size, ", cur_index: ", cur_index);
  return cur;
}

Token TokenStream::ParseChar() {
  // @TODO: @FIXME: fix / actually do this
  Unreachable("TODO");
}

Token TokenStream::Parse() {
  Token cur;
  cur.type = Token::Undefined;

  if (cur_index == read_size) Read();
  SkipWs();

  BeginLineRange();

  // post or precondition of ws could mean IsEOF()
  if (IsEOF()) {
    cur = Token(Token::EndOfFile, EndLineRange());
    return cur;
  }

  // parse complicated tokens
  switch (read_buf[cur_index]) {
    case '"':
      cur = ParseStr();
      break;
    case '\'':
      cur = ParseChar();
      break;
    default:
      cur = ParseNum();
      break;
  }

  // if we have a token
  if (cur.type != Token::Undefined) return cur;
  // else it is either a comment, a ident, or a operator/symbol
  cur = ParseSimpleToken();

  if (cur.type == Token::LineComment) {
    // read till \n
    cur.data = ParseLineComment();
    cur.pos = EndLineRange(-1);
  } else if (cur.type == Token::BlockComment) {
    // read till */
    auto comment = ParseBlockComment();
    if (!comment) {
      // missing `*/`
      return cur = Token(Token::Undefined, EndLineRange());
    }
    cur.data = *comment;
    cur.pos = EndLineRange(-1);
  }

  if (cur.type == Token::Undefined) cur = ParseId();
  Assert(cur.type < Token::NumTokens, "cur type must be valid",
         static_cast<int>(cur.type));

  return cur;
}

std::optional<int> TokenStream::ParseSimpleNumber(int max_digits, int base) {
  Assert(max_digits > 0, "Must be positive and not 0", max_digits);
  Assert(base > 0, "Must be positive and not 0", base);
  std::string cur;
  int cur_digits = 0;

  while (cur_digits < max_digits) {
    if (cur_index == read_size) Read();
    // I guess this could be valid (though I don't see how since it won't have a
    // string ender) Still should be rigorous I guess
    if (IsEOF()) break;
    if (read_buf[cur_index] >= '0' && read_buf[cur_index] <= '9') {
      cur_digits++;
      cur.push_back(read_buf[cur_index]);
      cur_index++;
      col++;
    } else {
      // reached invalid char
      // (definitely can be a viable string for example \na is just `\n` and
      // `a`)
      // @INFO: We could support `_` in numbers here too????
      break;
    }
  }

  if (cur_digits == 0) return std::nullopt;
  return std::stoi(cur, 0, base);
}

// returns str since we may return multiple unicode chars for a single codepoint
void TokenStream::ConvEscapeCodes(std::string &str) {
  col++;
  switch (read_buf[cur_index++]) {
    case 'n':
      str.push_back('\n');
      break;
    case 'r':
      str.push_back('\r');
      break;
    case 't':
      str.push_back('\t');
      break;
    case 'v':
      str.push_back('\v');
      break;
    case 'a':
      str.push_back('\a');
      break;
    case 'b':
      str.push_back('\b');
      break;
    case 'f':
      str.push_back('\f');
      break;
    case 'x': {
      // hex
      auto num = ParseSimpleNumber(3, 16);
      if (num)
        str.push_back(static_cast<unsigned char>(*num));
      else {
        // @TODO: error
        std::cerr << "Invalid hex str lit!!!  TODO: better errors" << std::endl;
      }
    } break;
    case 'u': {
      // small unicode
      // @TODO: (I would like a stdlib way the other way is ugh)
      Unreachable("Unreachable, TODO");
    } break;
    case 'U': {
      // large unicode
      // @TODO: (I would like a stdlib way the other way is ugh)
      Unreachable("Unreachable, TODO");
    } break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      // octal
      // a bit ugh; we have to manually fix it up
      // since \01 is the octal 01 and we need to include
      // the first digit (for example \232).
      // @TODO: @CLEANUP: possibly what we do is force a 0 at the front
      //                  for example instead of \232 it is \0232 you can
      //                  still just do \0 if you want
      //                  though it would also make it cleaner
      cur_index--;
      col--;
      auto num = ParseSimpleNumber(3, 8);
      if (num)
        str.push_back(static_cast<unsigned char>(*num));
      else {
        // @TODO: error
        std::cerr << "Invalid oct str lit!!!  TODO: better errors" << std::endl;
      }
    } break;
    // @TODO: currently this allows stuff like; `\k` where it just
    //        outputs `k`, this is not suitable and really should error
    //        but since errors aren't handled well by the tokenizer yet
    //        I'm gonna push that back
    default:
      str.push_back(read_buf[cur_index]);
  }
}

Token TokenStream::ParseStr() {
  Token cur;
  char *buf = &read_buf[cur_index];
  Assert(buf[0] == '"', "Expecting buf to begin with \"", buf[0]);

  buf++;
  size_t len = read_size - cur_index - 1;
  size_t i = 0;
  bool escaped = false;
  std::string str = std::string();

  for (;;) {
    // in some cases we may need to grab more from file
    // for big strings or awkard places where the string is
    // towards the end of the 1024 boundary
    if (i == len) {
      Read();
      if (IsEOF()) {
        // if we are EOF then just set the undefined token and ret
        cur = Token(Token::Undefined, EndLineRange());
        return cur;
      }

      // reset
      i = 0;
      buf = read_buf;
      len = read_size;
    }
    if (buf[i] == '\\') {
      escaped = true;
    } else if (buf[i] == '"' && !escaped) {
      break;
    } else {
      if (!escaped)
        str.push_back(buf[i]);
      else
        ConvEscapeCodes(str);
      escaped = false;
    }
    i++;
  }

  // +2 since +1 for each `"`
  col += i + 2;
  cur_index += i + 2;
  cur = Token(Token::Str, EndLineRange(-1), str);
  return cur;
}

bool valid_char(char c) {
  return (c >= '0' && c <= '9') ||  // digit
         c == 'e' || c == 'E' ||    // exp
         c == '.' || c == '_';      // dec pt and seperator
}

bool is_num(char c) { return c >= '0' && c <= '9'; }

Token TokenStream::ParseNum() {
  char *buf = &read_buf[cur_index];
  Token cur;

  // @NOTE: this doesn't handle hex
  // @CLEANUP: this needs to be redone

  /*
    @TEST:
    Not a number (flt/int) if:
    1) first char is not a digit, not 'e' not 'E' not '.'
    2) first char is '.' second char not a digit
    Note: Not allowed `_` to avoid any of the cases above
  */

  if (!is_num(buf[0])) {
    if (cur_index >= read_size - 2) {
      ReadOffset();
      buf = read_buf;
    }

    if (buf[0] != '.' || !is_num(buf[1])) {
      cur = Token(Token::Undefined, EndLineRange());
      return cur;
    }
  }

  std::string str = std::string();
  size_t i = 0;
  size_t len = read_size - cur_index;
  bool handled_exp = false;
  bool handled_dot = false;
  bool prev_exp = false;

  // sign is separate and not handled here
  // just parsing a sequence of digits
  for (;;) {
    if (i >= len) {
      Read();
      // EOF is valid so just break
      if (IsEOF()) break;
      // reset
      i = 0;
      buf = read_buf;
      len = read_size - cur_index;
    }

    if (prev_exp) {
      int old_i = i - 1;
      if (buf[i] == '_') {
        i++;
        continue;  // skip `_`
      }

      if (buf[i] == '+' || buf[i] == '-' || is_num(buf[i])) {
        str.push_back(buf[i]);
        handled_exp = true;
        prev_exp = false;
        i++;
        continue;
      } else {
        // @WEIRD: previously we tried to adjust for the E
        //         but syntatically you can't have an 'e'
        //         after a number and have it be valid unless
        //         it is scientific notation.
        //         so easier to error.
        cur = Token(Token::Undefined, EndLineRange());
        return cur;
      }
    }

    if (!valid_char(buf[i])) break;

    // ignoring the '_' and just not adding it
    // @TODO: we want to only allow one seperator
    if (buf[i] >= '0' && buf[i] <= '9') {
      str.push_back(buf[i]);
    } else if (buf[i] == '.') {
      // you can't have 4e2.2 for example
      // so we have to stop on the dot if we
      // have handled the scientific
      if (handled_dot || handled_exp) break;
      handled_dot = true;
      str.push_back(buf[i]);
    } else if (buf[i] == 'e' || buf[i] == 'E') {
      if (handled_exp) break;
      prev_exp = true;
      str.push_back(buf[i]);
    }
    i++;
  }

  if (!handled_exp && prev_exp) {
    // we didn't handle 'e' properly (EOF)
    // but you can't have this be valid syntax
    // so we can just error out on it.
    cur = Token(Token::Undefined, EndLineRange());
    return cur;
  }

  col += i;
  cur_index += i;
  cur.pos = EndLineRange(-1);

  if (handled_dot || handled_exp) {
    cur.type = Token::Flt;
    cur.data = std::stof(str);
  } else {
    cur.type = Token::Int;
    cur.data = static_cast<std::int64_t>(std::stol(str));
  }
  return cur;
}

bool valid_id_char_starting(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool valid_id_char(char c) {
  return valid_id_char_starting(c) || (c >= '0' && c <= '9');
}

Token TokenStream::ParseId() {
  if (!valid_id_char_starting(read_buf[cur_index])) return Token();

  Token cur;
  std::string str = std::string();

  do {
    str.push_back(read_buf[cur_index++]);
    col++;
    if (cur_index == read_size) Read();
  } while (valid_id_char(read_buf[cur_index]));

  cur.type = Token::Identifier;
  cur.data = str;
  cur.pos = EndLineRange(-1);
  return cur;
}

void TokenStream::Read() {
  read_size = reader->Read(static_cast<char *>(read_buf), TokenizerBufSize);
  cur_index = 0;
}

void TokenStream::ReadOffset() {
  size_t offset = read_size - cur_index;
  memmove(static_cast<char *>(read_buf),
          static_cast<char *>(read_buf) + cur_index, read_size - cur_index);
  read_size = reader->Read(static_cast<char *>(read_buf) + offset,
                           TokenizerBufSize - offset);
  cur_index = 0;
  read_size += offset;
}

}  // namespace porc