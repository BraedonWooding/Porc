# Generates token data based upon the tokens file
# Information about file format is at the top of the tokens file

# f-string expressions can't include backslashes so we define them here
# also allows us to change tab to `\t` easily if we ever need to
tab = '    '
null_term = '\\0'
split_on = '\t'
file_location = "src/tokens"

# Effectively the same structure as the _token_set_t struct
# Except it uses dictionaries/hashtables (more efficient in python and
# takes less space and easier to use).
class TokenSet:
    # tokens contains all the tokens for single characters
    # child_tokens shows the branching for characters
    """
        i.e.
          > (token)
        - (token)
          - (token)
          = (token)
        The `-` would be in .tokens
        The `-` would also be in .child_tokens which would have a sub tokenset
        with `>`, `-`, `=` all as .tokens of that child subset
    """

    # debugging purposes
    def __str__(self):
        return "tokens: " + str(self.tokens) + "\n" + "child_tokens: " + \
        '\n'.join([str(key) + ": " + str(val) for key, val in self.child_tokens.items()])

    def __init__(self):
        self.tokens = {}
        self.child_tokens = {}

    # Recursively adds token to children one character at a time
    # Till the last character where it goes into the self.tokens dictionary
    def add_token(self, key, token):
        if len(key) == 1:
            self.tokens[key] = token
        else:
            if key[0] not in self.child_tokens:
                self.child_tokens[key[0]] = TokenSet()
            self.child_tokens[key[0]].add_token(key[1:], token)

# Prints out token maps recursively
def print_token(file, token, counter):
    original_counter = counter
    counter += 1
    # Grab the bits we care about
    children_nodes = [(key, tok) for key, tok in token.child_tokens.items()]
    token_nodes = [(key, tok) for key, tok in token.tokens.items()]
    if len(token_nodes) == 0:
        file.write(f"{tab * (counter - 1)}NULL,\n")
    else:
        file.write(f"{tab * (counter - 1)}(const int[ASCII_SET]){{\n")
        for node in token_nodes:
            file.write(f"{tab * counter}['{node[0]}'] = TOK_{node[1]},\n")
        file.write(f"{tab * (counter - 1)}}},\n")

    if len(children_nodes) == 0:
        file.write(f"{tab * (counter - 1)}NULL,\n")
    else:
        file.write(f"{tab * (counter - 1)}(const struct _token_set_t*[ASCII_SET]){{\n")
        for node in children_nodes:
            file.write(f"{tab * counter}['{node[0]}'] = (const struct _token_set_t[1]){{\n")
            print_token(file, node[1], counter + 1)
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

        toks = [x.strip() for x in line.split(split_on) if x]
        if toks[0] == "COMMA": toks = ["COMMA", '","']

        token_list.write(f"TOK_{toks[0]},\n")
        string_to_write += f"\n{tab}[TOK_{toks[0]}] = \"{toks[0]}\","
        if len(toks) >= 2:
            token_data.write(f"{tab}[TOK_{toks[0]}] = {toks[1]},\n")
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
    with open(file_location, "r") as f:
        generate(f)

if __name__ == "__main__":
    main()
