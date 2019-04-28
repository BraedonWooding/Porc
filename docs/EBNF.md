# EBNF

## Actual grammar

This is more to make sure we something to make sure our parser is consistent.

A few notes:

- `[a]` is identical to `(a)?` (i.e. 0 or 1)
- `(a)+` means `a` repeated (1+)
- `(a)*` means `a` repeated or not (0+)
- Basically identical to `[(a)+]` (which is `((a)+)?`)
- i.e. `if (x) 1 else 2` is identical to `if (x) return 1 else return 2;` and `return if (x) 1 else 2;`
- Rules should never define required symbols for that rule to be formed those should be outside i.e. a block shouldn't include the `{` `}` required for it's definition they should be defined outside the block
- Semicolons aren't really parsed like this, basically you can leave them out whenever you use `{}` or when it is the last statement in a block (in that case it auto returns that from the expr).
  - Also we take the case of returning values in ambiguous cases; `x = () => 1;` is unclear if it returns a value or not but we will return the `1` unless the return type speaks differently.

TODO:

- Is `x..y:z <| f()` parsed correclty as `f(x..y:z)`?
- Folding expressions `|>` and `<|` are parsed incorrectly I think??
  - I think I want `g() <| x + 2 == 4 <| f()` parsed as `g(x + 2) == f(4)`

```ebnf
// Fragments

Letter ::= 'a'-'z' | 'A'-'Z'
Digit ::= '0'-'9'
Sign ::= '+' | '-'
DigitSeq ::= Digit (Digit | '_')*
Identifier ::= (Letter | '_') | (Letter | Digit | '_')*
Int ::= DigitSeq
Float ::= FloatDecimal FloatExponent?
    | DigitSeq FloatExponent
// @TODO: too lazy to write this out properly
// just typical string rules with /" and so on
String ::= " ANY* "
Char ::= ' ANY '
Bool ::= 'true' | 'false';

// @Q: do we want to allow `0.`
FloatDecimal ::= DigitSeq? '.' DigitSeq
            | DigitSeq '.'

FloatExponent ::= ('e' | 'E') [Sign] DigitSeq

// Operators

AssignmentOp ::= '=' | '*=' | '/=' | '**=' | '%/=' | '%=' | '+=' | '-='

// - `block` means a scope definition
// - `decl` stands for declaration
// - `expr` stands for expression
// - `list` is translated to an array construct

// == Blocks ==

struct_block
  : var_decl ';'
  | func_decl
  | struct_decl
  | macro_expr [';']
  ;

file_decl
  : func_block*
  ;

func_block
// you can leave it out for auto return
// also don't need it for control flow exprs
  : func_decl
  | struct_decl
  | assignment_expr ';'
  | ['return'] expr [';']
  ;

if_block
  : 'if' expr func_block ('else if' expr func_block)* ['else' func_block]
  ;

for_block
// we have to explicitly allow the '(' ')'
// since the inside statement isn't an expr
  : 'for' '(' identifier_list 'in' expr_list ')' func_block
  | 'for' identifier_list 'in' expr_list func_block
  ;

while_block
  : 'while' expr func_block
  ;

// == Declarations ==

tuple_value_decl
  : tuple_type_decl ['=' expr]
  ;

tuple_type_decl
  : ['mut'] ['$'] Identifier [':' type_expr]
  ;

tuple_decl
  : '(' [tuple_value_decl (',' tuple_value_decl)*] ')'
  ;

var_decl
  : ['const' | 'mut'] identifier_list ':' type_expr_list '=' expr_list
  | ['const' | 'mut'] identifier_list '=' expr_list
  | ['const' | 'mut'] identifier_list ':' type_expr_list
  ;

// == Expressions ==

macro_expr
  : '@' identifier_access '(' expr_list? ')'
  ;

generic_type_decl
  : constant
  | type_expr
  ;

type_expr
  : '(' tuple_decl ')'
  | 'fn' Identifier? tuple_decl ['->' type_expr]
// @TODO: decide if we want to just allow idents or if we want to allow type exprs
//        for example `Array[int] | List[int]` vs `(Array | List)[int]`
//        I'm leaning towards the first but I'm not sure
// @NOTE: I'm pretty sure I want the first I don't see a use for the second.
//          It is just too specific
  | '$' identifier
  | identifier_access '[' generic_type_decl (',' generic_type_decl)+ ']'
  | identifier_access
  | type_expr ('|' type_expr)+
  ;

assignment_expr
  : expr_list AssignmentOp expr_list
  ;

expr
  : logical_or_expr                // arithmetic
  | var_decl
  | logical_or_expr '..' '='? logical_or_expr [':' logical_or_expr] // range
  | anonymous_struct_decl
  | lambda_decl
  | map_expr | array_expr | tuple_expr
  | if_block | while_block | for_block
  | '{' func_block* '}'
  ;

lambda_decl
  : tuple_decl ['->' type_expr] '=>' func_block
  ;

func_decl
  : 'fn' Identifier tuple_decl ['->' type_expr] '{' func_block* '}'
  ;

anonymous_struct_decl
  : tuple_decl '::' '{' struct_block* '}'
  ;

struct_decl
  : 'struct' Identifier tuple_decl '{' struct_block* '}'
  ;

tuple_expr
  : '(' ')'
  | '(' expr ',' ')'
  | '(' expr_list ')' // has to have 2 or more (otherwise will match above)
  ;

array_expr
  : '[' expr_list ']'
  ;

map_expr
  : '[' expr_pair_list ']'
  ;

func_call
  : atom_expr '(' expr_list ')
  ;

atom
  : '(' expr ')'
  | '@' identifier_access '(' expr_list ')'
  | expr '|>' func_call
  | func_call '<|' expr
  | func_call
  // i.e. array[1] or array[1:] or array[1::] or array[1:2:]
  // or array[] (lexical error) or array[:2:] ... and so on
  | atom '[' expr? [ ':' expr? [':' expr?] ] ']'
  | atom '.' identifier_access
  | Identifier
  | constant
  ;

power_expr
  : atom ('**' atom)*

unary_expr
  : ('+' | '-' | '!')* power

multiplicative_expr
  : power_expr (('*' | '/' | '%' | '//') power_expr)*

additive_expr
  : multiplicative_expr (('+' | '-') multiplicative_expr)*
  ;

comparison_expr
  : additive_expr (('<' | '>' | '>=' | '<=' | '==' | '!=') additive_expr)*
  | additive_expr 'is' type_expr
  ;

logical_and_expr
  : comparison_expr ('&&' comparison_expr)*
  ;

logical_or_expr
  : logical_and_expr ('||' logical_and_expr)*
  ;

constant
  : Float | Int | String | Char | Bool
  ;

// == Lists ==

identifier_access
  : Identifier ('.' Identifier)*
  ;

identifier_access_list
  : identifier_access (',' identifier_access)*
  ;

identifier_list
  : Identifier (',' Identifier)*
  ;

type_expr_list
  : type_expr (',' type_expr)*
  ;

expr_list
  : expr (',' expr)*
  ;

expr_pair_list
  : expr ':' expr (',' expr ':' expr)*

var_decl_list
  : var_decl (',' var_decl)*
  ;
```
