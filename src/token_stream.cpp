#include "token_stream.hpp"

#include <string>
#include <locale>
#include <optional>
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
  - We have to be wary of the boundary reads i.e. ['\']['\'] has to be passed correctly
    - A few helpful functions; Read/ReadAll
  - We also have to adjust col/line appropriately but that is pretty easy
    and mostly covered by the parse functions.
  - We do it this way since it means a few things;
    - More efficient/Usable on memory sparce systems (low amount of ram)
    - Better IO pipelining (consistently small reads is better than
      one large read).
    - Supports insanely large files better
    - Doesn't cause huge segmentation (this really only effects interpreters which we are)
      - This is because if we allocate a huge memory block before we are even running
        the program then deallocating before we run it we could possibly cause some
        segmentation (of course in reality large blocks of segmentation are rarely an issue)

  @NOTE:  make sure you are using fadvise (FADV_NOREUSE | FADV_SEQUENTIAL)
      to get the most performance out of this way of reading the files.

  @TODO:  decouple these functions
*/

namespace porc::internals {

void TokenStream::ReadAll() {
  Read(BufSize, 0);
}

inline void TokenStream::BeginLineRange() {
  old_line = line;
  old_col = col;
}

inline LineRange TokenStream::EndLineRange(int col_offset) const {
  return LineRange(old_line, line, old_col, col + col_offset, GetFileName());
}

Token TokenStream::PeekCur() {
  if (cur_token_size == 0) Next();
  return tokens[cur_token_size - 1];
}

Token TokenStream::PopCur() {
  if (cur_token_size == 0) Next();
  Token tok = tokens[cur_token_size - 1];
  cur_token_size--;
  return tok;
}

void TokenStream::Push(Token tok) {
  Assert(cur_token_size < MaxLookaheads,
         "Can't `Push()` consecutively more than `MaxLookaheads`",
         MaxLookaheads);
  tokens[cur_token_size++] = tok;
}

bool TokenStream::Next() {
  Assert(cur_token_size < MaxLookaheads,
         "Can't `Next()` consecutively more than `Lookaheads`",
         MaxLookaheads);
  Token tok = Parse();
  tokens[cur_token_size++] = tok;
  return tok;
}

void TokenStream::SkipWs() {
  Assert(IsEOF() || cur_index < read_size,
         "Precondition failed",
         "cur_index: ", cur_index, ", read_size: ", read_size);

  while(!IsEOF() && std::isspace(read_buf[cur_index])) {
    if (read_buf[cur_index] == '\n') {
      line++;
      col = 0;
    } else {
      col++;
    }
    cur_index++;
    if (cur_index == read_size) ReadAll();
  }

  Assert(cur_index < read_size || IsEOF(),
         "Postcondition failed",
         "cur_index: ", cur_index, read_size, ", read_size: ", read_size);
}

std::string TokenStream::ParseLineComment() {
  std::string buf;
  while (true) {
    if (cur_index == read_size) ReadAll();
    if (IsEOF() || read_buf[cur_index] == '\n') break;
    buf.push_back(read_buf[cur_index++]);
    col++;
  }
  if (read_buf[cur_index] == '\n') {
    col = 0;
    line++;
  }

  return buf;
}

bool TokenStream::BufMatches(std::string str) {
  for (int i = 0; i < str.size(); i++) {
    if (cur_index == read_size) ReadAll();
    if (IsEOF()) return false;
    if (str[i] != read_buf[cur_index]) return false;
    // @TODO: do we want to also increment line if we reach the end
    //        I don't think our str can have newlines and behave properly
    //        but we don't want weird things like that occurring
    cur_index++;
    col++;
  }
  return true;
}

// supports nesting
std::optional<std::string> TokenStream::ParseBlockComment() {
  std::string buf;
  int depth = 1;

  while (depth > 0) {
    if (cur_index == read_size) ReadAll();
    // block comment not ended
    if (IsEOF()) return std::nullopt;
    if (read_buf[cur_index] == '*') {
      // since we have read all already at the top
      // we are sure that cur_index must be < read_size
      // else its an EOF
      cur_index++;
      col++;
      if (cur_index == read_size) ReadAll();
      if (IsEOF()) return std::nullopt;

      if (read_buf[cur_index] == '/') {
        depth--;
        col++;
      } else {
        buf.push_back(read_buf[cur_index]);
        if (read_buf[cur_index] == '\n') {
          col = 0;
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
          col = 0;
          line++;
        }
      }
      cur_index++;
    } else {
      buf.push_back(read_buf[cur_index]);
      if (read_buf[cur_index] == '\n') {
        col = 0;
        line++;
      }
    }
  }
  return buf;
}

Token TokenStream::ParseSimpleToken() {
  char *buf = &read_buf[cur_index];
  size_t len = read_size - cur_index;
  Token cur;
  cur.type = Token::Undefined;

  // @OPTIMISATION: these will be more common so maybe we should put these above the others
  // or we should see if we can jump straight to the first token easily
  const TokenSet *current_set = &tokenFromStrMap;
  const TokenSet *previous_set = NULL;
  int i = 0;

  // Recursively call on each child token set till we reach a dead end
  // either we can go back one level and step into the 'value' section of the tree
  // or we can't and thus no token can be found. i.e. `+>` will parse the `+` and `>` separately
  // `+=>` will parse as `+=` and `>`.  Of course this may not be preferred so we may want to
  // have a precendence for tokens, I think it is fine though for now.
  while (true) {
    if (cur_index + i == read_size) {
      Assert(cur_index != 0, "We can't have a token with a length more than BufSize", cur_index);
      // This is incase I ever change things and break this offcase
      // We should never get to this point and to not have iterated i
      // Since that would mean that our buf is full which can't be the case at this pt
      // if we have not read using i.
      Assert(i > 0, "Buffer shouldn't be full if i == 0", i);

      // shuffle the important bits down the the front of the buffer
      memmove(static_cast<char*>(read_buf),
              static_cast<char*>(read_buf) + cur_index, i);

      // now we can read into the buffer
      Read(read_size - i, i);
      buf = read_buf;
      len = read_size;

      if (IsEOF()) {
        // Our token wasn't finished
        // @TEST: This may trigger incorrectly if the last token is something like
        // `cont` or similar.
        // @POSSIBLE_FIX: I think I have fixed it as the identifier will grab
        // it if it can be regarded as an identifier/number, again test/check
        read_buf[i + 1] = 0;
        std::cerr << "Possibly incorrect trigger read text so far is " << read_buf + cur_index << std::endl;
        col += i + 1;
        cur = Token(Token::Undefined, EndLineRange(-1));
        break;
      }
    }

    // trie leaf
    // @NOTE: you can't have WS in tokens that is a token can't contain a line break
    // or any kinda of ws in the middle of it i.e = > is always going to be `=` and `>` and never `=>`.
    // This statement allows us to just increment col as expected.
    if (current_set->child_tokens == NULL || (current_set->child_tokens[buf[i]].child_tokens == NULL && 
                          current_set->child_tokens[buf[i]].tokens == NULL)) {
      // We hit a dead end but can we find a value node either in our current node
      // or go back one token
      if (current_set->tokens != NULL && current_set->tokens[buf[i]] != static_cast<int>(Token::Undefined)) {
        col += i + 1;
        cur = Token(static_cast<Token::Kind>(current_set->tokens[buf[i]]),
                    EndLineRange(-1));
        cur_index += i + 1;
      } else {
        // i.e. parse `<!` as `<` and `!`
        if (previous_set != NULL && previous_set->tokens != NULL && previous_set->tokens[buf[i - 1]] != static_cast<int>(Token::Undefined)) {
          col += i;
          cur = Token(static_cast<Token::Kind>(previous_set->tokens[buf[i - 1]]),
                      EndLineRange(-1));
          cur_index += i;
        }
      }
      break;
    } else {
      // go deeper into set
      previous_set = current_set;
      current_set = &current_set->child_tokens[buf[i]];
      i++;
    }
  }

  Assert(IsEOF() || cur_index < read_size,
         "Postcondition Failed",
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

  if (cur_index == read_size) ReadAll();
  SkipWs();

  BeginLineRange();

  // post or precondition of ws could mean IsEOF()
  if (IsEOF()) {
    cur = Token(Token::EndOfFile, EndLineRange());
  }

  // parse complicated tokens
  switch (read_buf[cur_index]) {
    case '"':   cur = ParseStr();   break;
    case '\'':  cur = ParseChar();  break;
    default:    cur = ParseNum();   break;
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
    cur.data = comment.value();
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
    if (cur_index == read_size) ReadAll();
    // I guess this could be valid (though I don't see how since it won't have a string ender)
    // Still should be rigorous I guess
    if (IsEOF()) break;
    if (read_buf[cur_index] >= '0' && read_buf[cur_index] <= '9') {
      cur_digits++;
      cur.push_back(read_buf[cur_index]);
      cur_index++;
      col++;
    } else {
      // reached invalid char
      // (definitely can be a viable string for example \na is just `\n` and `a`)
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
  switch(read_buf[cur_index++]) {
    case 'n': str.push_back('\n'); break;
    case 'r': str.push_back('\r'); break;
    case 't': str.push_back('\t'); break;
    case 'v': str.push_back('\v'); break;
    case 'a': str.push_back('\a'); break;
    case 'b': str.push_back('\b'); break;
    case 'f': str.push_back('\f'); break;
    case 'x': {
      // hex
      auto num = ParseSimpleNumber(3, 16);
      if (num) str.push_back(static_cast<unsigned char>(num.value()));
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
    case '0': case '1': case '2': case '3': case '4': case '5': case '6':
    case '7': case '8': case '9': {
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
      if (num) str.push_back(static_cast<unsigned char>(num.value()));
      else {
        // @TODO: error
        std::cerr << "Invalid oct str lit!!!  TODO: better errors" << std::endl;
      }
    } break;
    // @TODO: currently this allows stuff like; `\k` where it just
    //        outputs `k`, this is not suitable and really should error
    //        but since errors aren't handled well by the tokenizer yet
    //        I'm gonna push that back
    default: str.push_back(read_buf[cur_index]);
  }
}

Token TokenStream::ParseStr() {
  Token cur;
  char *buf = &read_buf[cur_index];
  Assert(buf[0] == '"', "Expecting buf to begin with \"", buf[0]);

  buf++;
  size_t len = read_size - cur_index - 1;
  int i = 0;
  bool escaped = false;
  std::string str = std::string();

  for (;;) {
    // in some cases we may need to grab more from file
    // for big strings or awkard places where the string is
    // towards the end of the 1024 boundary
    if (i == len) {
      ReadAll();
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
      if (!escaped)   str.push_back(buf[i]);
      else            ConvEscapeCodes(str);
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
  return (c >= '0' && c <= '9') || // digit
      c == 'e' || c == 'E'  || // exp
      c == '.' || c == '_';    // dec pt and seperator
}

bool is_num(char c) {
  return c >= '0' && c <= '9';
}

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

  // Super ugly, should be hoistered into callsite
  // or we should just exit gracefully???
  if (!is_num(buf[0])) {
    if (cur_index >= read_size - 2) {
      // since we need at most three to see
      // should be rare enough that this doesn't matter
      Read(BufSize - 2, cur_index + 2);
      buf = read_buf;
      cur_index = 0;
    }

    if (buf[0] != '.' || !is_num(buf[1])) {
      cur = Token(Token::Undefined, EndLineRange());
      return cur;
    }
  }

  std::string str = std::string();
  int i = 0;
  size_t len = read_size - cur_index;
  bool handled_exp = false;
  bool handled_dot = false;
  bool prev_exp = false;

  // sign is separate and not handled here
  // just parsing a sequence of digits
  for (;;) {
    if (i >= len) {
      ReadAll();
      // EOF is valid so just break
      if (IsEOF()) break;
      // reset
      i = 0;
      buf = read_buf;
      len = read_size;
    }

    if (prev_exp) {
      int old_i = i - 1;
      if (buf[i] == '_') continue; // skip `_`

      if (buf[i] == '+' || buf[i] == '-' || is_num(buf[i])) {
        str.push_back(buf[i]);
        handled_exp = true;
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
  if (!valid_id_char_starting(read_buf[cur_index]))
    return Token();

  Token cur;
  std::string str = std::string();
  str.push_back(read_buf[cur_index++]);
  col++;

  while (valid_id_char(read_buf[cur_index])) {
    str.push_back(read_buf[cur_index++]);
    col++;
  }

  cur.type = Token::Identifier;
  cur.data = str;
  cur.pos = EndLineRange(-1);
  return cur;
}

void TokenStream::Read(uint len, uint offset) {
  read_size = reader->Read(static_cast<char*>(read_buf) + offset, len);
  cur_index = offset;
  read_buf[offset + len] = '\0';
}

}