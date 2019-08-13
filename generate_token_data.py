# Generates token data based upon the tokens file
# Information about file format is at the top of the tokens file

# f-string expressions can't include backslashes so we define them here
# also allows us to change tab to `\t` easily if we ever need to
tab = '  '
null_term = '\\0'
split_on = ' '
file_location = "src/tokens"

def generate(file):
    token_list = open("src/token_list.inc", "w")
    token_data = open("src/token_data.hpp", "w")
    token_data_impl = open("src/token_data.cpp", "w")

    # token_constants_def = open("src/token_constants_definitions.inc", "w")
    # token_constants_vals = open("src/token_constants.inc", "w")
    token_list.write("/* Auto Generated File */\n")
    # token_constants_def.write("/* Auto Generated File */\n")
    # token_constants_vals.write("/* Auto Generated File */\n")
    token_data.write(
"""/* Auto Generated File */
#ifndef TOKEN_DATA_HPP
#define TOKEN_DATA_HPP

#include "token.hpp"

namespace porc {
  const char *tokenToStr(Token::Kind token);
  const char *tokenToName(Token::Kind token);
  Token::Kind tokenFromStr(std::string_view str);
}

#endif
"""
    )
    token_data_impl.write(
"""/* Auto Generated File */
#include "token_data.hpp"

namespace porc {
\n"""
    )

    token_data_impl.write("const char *tokenToStr(Token::Kind token) {\n" + tab + "switch (token) {\n")
    string_to_write = "const char *tokenToName(Token::Kind token) {\n" + tab + "switch (token) {"

    tokens = []

    for line in file:
        # starts with
        if line[0:2] == "//" or not line.strip(): continue

        toks = [x.strip() for x in line.split(split_on) if x]
        if toks[0] == "COMMA": toks = ["COMMA", '","']

        token_list.write(f"{toks[0]},\n")
        # token_constants_def.write(f"static const Token {toks[0]};\n")
        # token_constants_vals.write(f"const Token Token::{toks[0]} = Token(Token::Kind::{toks[0]}, LineRange::Null());\n")
        string_to_write += f"{tab + tab}case Token::Kind::{toks[0]}: return \"{toks[0]}\";\n"
        if len(toks) >= 2:
            token_data_impl.write(f"{tab + tab}case Token::Kind::{toks[0]}: return {toks[1]};\n")
            token = toks[1].strip('"')
            # handle the case of just ws properly
            if not token: token = " "
            tokens.append((token, toks[0]))
    token_data_impl.write(f"{tab + tab}default: return nullptr;\n{tab}}}\n}}\n\n" + string_to_write + f"{tab + tab}default: return nullptr;\n{tab}}}\n}}\n\n")

    token_data_impl.write(f"Token::Kind tokenFromStr(std::string_view str) {{\n")
    for tok in tokens:
        token_data_impl.write(f"{tab}if (str == \"{tok[0]}\") return Token::Kind::{tok[1]};\n")
    token_data_impl.write(f"{tab}return Token::Kind::Undefined;\n}}\n")

    token_data_impl.write("}\n")
    token_data_impl.close()
    token_list.close()
    token_data.close()
    # token_constants_vals.close()
    # token_constants_def.close()

def main():
    with open(file_location, "r") as f:
        generate(f)

if __name__ == "__main__":
    main()
