# Contributer Guide

> This is gonna be quite verbose before I come back and make it clearer, mostly just a mind dump for now sorry!

Hey!  This language was designed both to serve the purpose of being a better shell scripting language, show that C can still create modern programs and illustrate a few ideas I had for making a more modern C.

Hopefully this guide helps cover the few 'oddities'

## Compiler Pipeline

### Tokenization

You will notice that the two major token files are auto generated (`token_data.incl` and `token_list.incl`) these are generated automatically by `generate_token_data.py` using the `tokens` file.

Basically `token_list` just contains each token (comma separated) basically for the uses of creating an enum from them.

Whereas `token_data` contains conversions both from string to tokens and backwards (both the 'name' format and a more typical 'output format' i.e. name being `TOK_EQL` and 'output' being `=`) we use an in-memory trie for conversions (both for efficiency, ease of use and just simplicitly).

We store more than a typical tokenizer for example TOK_INT stores an int and TOK_FLT stores a floating point, TOK_STR and TOK_CHAR store strings/chars respectively, this mainly just makes parsing easier with the only caveat being that we have to have a special case for `.` that is if you have `.l` it is separate tokens (`.` and `l` - with any sequence of letters being identifiers) where as `.0` is a valid float, luckily it is pretty easy for us to determine valid floats and having just `.` is invalid as a float so it can't be the last character before EOF so we can just omit a syntax erorr in that case.


