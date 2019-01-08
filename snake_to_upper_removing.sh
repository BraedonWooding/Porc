#!/usr/bin/env bash

# Converts a_b_c to ABC
# The second argument is then filtered out
# i.e. `./snake_to_upper_removing "AST_OTHER_THING, AST_WOW" AST`
# $ OtherThing, Wow

echo "$1" | awk '{print tolower($0)}' | perl -CIO -pe 's/(?:\b|_)(\p{Ll})/\u$1/g' | sed "s/$2//"
