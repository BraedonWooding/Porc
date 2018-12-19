# Generates token data based upon the tokens.incl file
# f-string expressions can't include backslash
tab = '    '
null_term = '\\0'

class TokenSet:
    def __init__(self):
        self.tokens = {}
        self.child_tokens = {}

    def add_token(self, key, token):
        if len(key) == 1:
            self.tokens[key] = token
        else:
            if key[0] not in self.child_tokens:
                self.child_tokens[key[0]] = TokenSet()
            self.child_tokens[key[0]].add_token(key[1:], token)

def print_token(file, token, counter):
    original_counter = counter
    counter += 1
    second_bit = [(key, tok) for key, tok in token.child_tokens.items()]
    first_bit = [(key, tok) for key, tok in token.tokens.items()]
    if len(first_bit) == 0:
        file.write(f"{tab * (counter - 1)}NULL,\n")
    else:
        file.write(f"{tab * (counter - 1)}(const int[ASCII_SET]){{\n")
        for bit in first_bit:
            file.write(f"{tab * counter}['{bit[0]}'] = TOK_{bit[1]},\n")
        file.write(f"{tab * (counter - 1)}}},\n")

    if len(second_bit) == 0:
        file.write(f"{tab * (counter - 1)}NULL,\n")
    else:
        file.write(f"{tab * (counter - 1)}(const struct _token_set_t*[ASCII_SET]){{\n")
        for bit in second_bit:
            file.write(f"{tab * counter}['{bit[0]}'] = (const struct _token_set_t[1]){{\n")
            print_token(file, bit[1], counter + 1)
        file.write(f"{tab * (counter - 1)}}},\n")
    file.write(f"{tab * (original_counter - 1)}")
    file.write(f"}}{';' if original_counter == 1 else ','}\n")

def generate(file):
    token_list = open("src/token_list.incl", "w")
    token_data = open("src/token_data.incl", "w")
    token_list.write("/* Auto Generated File */\n")
    token_data.write("/* Auto Generated File */\n")

    token_data.write("static const char *tokenToStrMap[NUM_TOKENS] = {\n")
    string_to_write = "static const char *tokenToNameMap[NUM_TOKENS] = {"

    main_token = TokenSet()

    for line in file:
        # starts with
        if line[0:2] == "//" or not line.strip(): continue

        toks = [x.strip() for x in line.split('\t') if x]
        if toks[0] == "COMMA": toks = ["COMMA", '","']

        token_list.write(f"TOK_{toks[0]},\n")
        if len(toks) >= 2:
            token_data.write(f"{tab}[TOK_{toks[0]}] = {toks[1]},\n")
            string_to_write += f"\n{tab}[TOK_{toks[0]}] = \"{toks[0]}\","
            token = toks[1].strip('"')
            # handle the case of just ws properly
            if not token: token = " "
            main_token.add_token(token, toks[0])
    token_data.write("};\n\n" + string_to_write + "\n};\n\n")

    token_data.write(
    """typedef struct _token_set_t {
    const int *tokens;
    const struct _token_set_t **child_tokens;
} const *TokenSet;\n\n""")

    token_data.write(f"static const struct _token_set_t tokenFromStrMap = (struct _token_set_t){{\n")
    print_token(token_data, main_token, 1)

def main():
    with open("src/tokens", "r") as f:
        generate(f)

if __name__ == "__main__":
    main()
