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
    - A few helpful functions; Read/ReadAll, and BufMatches
    - BufMatches especially is really nice to check if the buffer contains a string
      and generally how it increments cur_index is what you want, you can always
      adjust it further if you really need to.
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

Token TokenStream::PeekCur() {
  if (tokens.size() == 0) Next();
  return tokens.at(tokens.size() - 1);
}

Token TokenStream::PopCur() {
  if (tokens.size() == 0) Next();

  Token tok = tokens.at(tokens.size() - 1);
  tokens.pop_back();
  return tok;
}

void TokenStream::Push(Token tok) {
  if (tokens.size() == MaxLookaheads)
    Unreachable("Can't `Push()` consecutively more than `Lookaheads`");

  tokens.push_back(tok);
}

bool TokenStream::Next() {
  if (tokens.size() == MaxLookaheads)
    Unreachable("Can't `Next()` consecutively more than `Lookaheads`");

  Token tok = Parse();
  tokens.push_back(tok);
  return tok;
}

void TokenStream::SkipWs() {
  Assert(read_size == 0 || cur_index < read_size,
         "Precondition failed",
         "cur_index: ", cur_index, ", read_size: ", read_size);

  while(read_size != 0 && std::isspace(read_buf[cur_index])) {
    if (read_buf[cur_index] == '\n') {
      line++;
      col = -1;
    }
    cur_index++;
    col++;
    if (cur_index == read_size) ReadAll();
  }

  Assert(cur_index < read_size || read_size == 0,
         "Postcondition failed",
         "cur_index: ", cur_index, read_size, ", read_size: ", read_size);
}

std::string TokenStream::ParseLineComment() {
  std::string buf;
  while (true) {
    if (cur_index == read_size) ReadAll();
    if (read_size == 0 || read_buf[cur_index] == '\n') break;
    buf.push_back(read_buf[cur_index++]);
  }
  return buf;
}

bool TokenStream::BufMatches(std::string str) {
  for (int i = 0; i < str.size(); i++) {
    if (cur_index == read_size) ReadAll();
    if (read_size == 0) return false;
    if (str[i] != read_buf[cur_index]) return false;
    cur_index++;
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
    if (read_size == 0) return std::nullopt;
    if (read_buf[cur_index] == '*') {
      if (BufMatches("*/")) {
        depth--;
        continue;
      } else {
        cur_index--;
      }
    } else if (read_buf[cur_index] == '/') {
      if (BufMatches("/*")) {
        depth++;
        continue;
      } else {
        cur_index--;
      }
    }
    buf.push_back(read_buf[cur_index++]);
  }
  return buf;
}

Token TokenStream::ParseSimpleToken() {
  char *buf = &read_buf[cur_index];
  size_t len = read_size - cur_index;
  Token cur;
  cur.type = Token::Kind::Undefined;

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
      memmove((char*)read_buf, (char*)read_buf + cur_index, i);

      // now we can read into the buffer
      Read(read_size - i, i);
      buf = read_buf;
      len = read_size;

      if (read_size == 0) {
        // Our token wasn't finished
        // @TEST: This may trigger incorrectly if the last token is something like
        // `cont` or similar.
        // @POSSIBLE_FIX: I think I have fixed it as the identifier will grab
        // it if it can be regarded as an identifier/number, again test/check
        read_buf[i + 1] = 0;
        std::cerr << "Possibly incorrect trigger read text so far is " << read_buf + cur_index << std::endl;
        cur = Token(Token::Kind::Undefined, LineRange(line, line, col, col + i));
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
      if (current_set->tokens != NULL && current_set->tokens[buf[i]] != static_cast<int>(Token::Kind::Undefined)) {
        cur = Token((Token::Kind)current_set->tokens[buf[i]], LineRange(line, line, col, col + i));
        cur_index += i + 1;
        col += i + 1;
      } else {
        // i.e. parse `<!` as `<` and `!`
        if (previous_set != NULL && previous_set->tokens != NULL && previous_set->tokens[buf[i - 1]] != static_cast<int>(Token::Kind::Undefined)) {
          cur = Token((Token::Kind)previous_set->tokens[buf[i - 1]], LineRange(line, line, col, col + i - 1));
          cur_index += i;
          col += i;
        }
      }
      // we want to just fall through if we don't get a nice token
      break;
    } else {
      // go deeper into set
      previous_set = current_set;
      current_set = &current_set->child_tokens[buf[i]];
      i++;
    }
  }

  Assert(read_size == 0 || cur_index < read_size, "Postcondition Failed", "read_size: ", read_size, ", cur_index: ", cur_index);
  return cur;
}

Token TokenStream::ParseChar() {
  Unreachable("TODO");
}

Token TokenStream::Parse() {
  // reset cur, we don't really care about 'resetting' data
  Token cur;
  cur.type = Token::Kind::Undefined;

  if (cur_index == read_size) ReadAll();
  SkipWs();

  // postcondition of ws could mean read_size == 0
  if (read_size == 0) return cur = Token::EndOfFile;

  // parse complicated tokens
  switch (read_buf[cur_index]) {
    case '"': cur = ParseStr(); break;
    case '\'': cur = ParseChar(); break;
    default: cur = ParseNum(); break;
  }

  if (cur.type != Token::Kind::Undefined) return cur;
  cur = ParseSimpleToken();

  if (cur.type == Token::Kind::LineComment) {
    // read till \n
    cur.data = ParseLineComment();
  } else if (cur.type == Token::Kind::BlockComment) {
    // read till */
    int old_line = line;
    int old_col = col;
    auto comment = ParseBlockComment();
    if (!comment) {
      // missing `*/`
      return cur = Token(Token::Kind::Undefined, LineRange(old_line, line, old_col, col));
    }
    cur.data = comment.value();
  }

  if (cur.type == Token::Kind::Undefined) cur = ParseId();
  Assert(cur.type < Token::Kind::NumTokens, "cur type must be valid",
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
    if (read_size == 0) break;
    if (read_buf[cur_index] >= '0' && read_buf[cur_index] <= '9') {
      cur_digits++;
      cur.push_back(read_buf[cur_index]);
      cur_index++;
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
  switch(read_buf[cur_index]) {
    case 'n': str.push_back('\n'); break;
    case 'r': str.push_back('\r'); break;
    case 't': str.push_back('\t'); break;
    case 'v': str.push_back('\v'); break;
    case 'a': str.push_back('\a'); break;
    case 'b': str.push_back('\b'); break;
    case 'f': str.push_back('\f'); break;
    case '0': case 'x': {
      // hex
      if (read_buf[cur_index] == '0') {
        if (!BufMatches("0x")) {
          // we are an octal
          // has to be duplicated since we have to cover the 'else' case differently
          auto num = ParseSimpleNumber(3, 8);
          str.push_back(num ? static_cast<unsigned char>(num.value()) : 0);
          return;
        }
      } else {
        cur_index++;
      }
      auto num = ParseSimpleNumber(2, 16);
      if (num) str.push_back(static_cast<unsigned char>(num.value()));
      else {
        // @TODO: error
        std::cerr << "Invalid str lit!!!  TODO: better errors" << std::endl;
      }
      return;
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
    case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
      auto num = ParseSimpleNumber(3, 8);
      if (num) str.push_back(static_cast<unsigned char>(num.value()));
      else {
        // @TODO: error
        std::cerr << "Invalid str lit!!!  TODO: better errors" << std::endl;
      }
      return;
    } break;
    default: str.push_back(read_buf[cur_index]);
  }
  // for the first set of things
  cur_index++;
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
  int old_line = line;
  int old_col = col;

  for (;;) {
    // in some cases we may need to grab more from file
    // for big strings or awkard places where the string is
    // towards the end of the 1024 boundary
    if (i == len) {
      ReadAll();
      if (read_size == 0) {
        // if we are EOF then just set the undefined token and ret
        cur.type = Token::Kind::Undefined;
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
  col += cur_index += i + 2;
  cur = Token(Token::Kind::Str, LineRange(old_line, line, old_col, col - 1), str);
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

  /*
    @TEST:
    Not a number (flt/int) if:
    1) first char is not a digit, not 'e' not 'E' not '.'
    4) first char is '.' second char not a digit
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
      cur.type = Token::Kind::Undefined;
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
      if (read_size == 0) break;
      // reset
      i = 0;
      buf = read_buf;
      len = read_size;
    }

    if (prev_exp) {
      if (buf[i] == '_') continue; // skip `_`

      if (buf[i] == '+' || buf[i] == '-' || is_num(buf[i])) {
        str.push_back(buf[i]);
        prev_exp = false;
        handled_exp = true;
        continue;
      } else {
        i--;
        break;
      }
    }

    if (!valid_char(buf[i])) break;

    // ignoring the '_' and just not adding it
    // @TODO: we want to only allow one seperator
    if (buf[i] >= '0' && buf[i] <= '9') {
      str.push_back(buf[i]);
    } else if (buf[i] == '.') {
      if (handled_dot) break;
      handled_dot = true;
      str.push_back(buf[i]);
    } else if (buf[i] == 'e' || buf[i] == 'E') {
      if (handled_exp) break;
      prev_exp = true;
      str.push_back(buf[i]);
    }
    i++;
  } 
  cur_index += i;

  if (handled_dot || handled_exp) {
    cur.type = Token::Kind::Flt;
    cur.data = std::stof(str);
  } else {
    cur.type = Token::Kind::Int;
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

bool not_end_block_comment(char *c) {
  // not very robust a bit hacky
  return (*c != '*' && c[1] != '/');
}

bool not_newline(char *c) {
  return *c != '\n';
}

Token TokenStream::ParseId() {
  if (!valid_id_char_starting(read_buf[cur_index]))
    return Token(Token::Kind::Undefined, LineRange::Null());

  Token cur;
  std::string str = std::string();
  str.push_back(read_buf[cur_index++]);

  while (valid_id_char(read_buf[cur_index])) {
    str.push_back(read_buf[cur_index++]);
  }
  cur.type = Token::Kind::Identifier;
  cur.data = str;
  return cur;
}

void TokenStream::Read(uint len, uint offset) {
  read_size = reader->Read(static_cast<char*>(read_buf) + offset, len);
  cur_index = offset;
  read_buf[offset + len] = '\0';
}

}