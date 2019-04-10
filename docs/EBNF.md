# EBNF

## Actual grammar

This is more to make sure we something to make sure our parser is consistent.

A few notes:

- `[a]` is identical to `(a)?` (i.e. 0 or 1)
- `(a)+` means `a` repeated (1+)
- `(a)*` means `a` repeated or not (0+)
- Basically identical to `[(a)+]` (which is `((a)+)?`)
- Semicolons are kinda implemented incorrectly here, basically the rules follow the 'expr language' return rules followed by languages like rust i.e. `return` returns but also the last statement which doesn't have a `;` (note this works inside if statements)
- i.e. `if (x) 1 else 2` is identical to `if (x) return 1 else return 2;` and `return if (x) 1 else 2;`
- Rules should never define required symbols for that rule to be formed those should be outside i.e. a block shouldn't include the `{` `}` required for it's definition they should be defined outside the block

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

file_block
  : var_decl ';'
  | 'fn' Identifier tuple_decl type_expr? '{' func_block* '}'
  | 'struct' Identifier tuple_decl '{' file_block* '}'
  | macro_expr ';'
  ;

func_block
  // you can leave it out for auto return
  // also don't need it for control flow exprs
  : ['return'] expr [';']
  | var_decl ';'
  | assignment_expr ';'
  ;

if_block
  : 'if' expr expr ('else if' expr expr)* ['else' expr]
  ;

for_block
  : 'for' '(' identifier_list 'in' expr_list ')' expr
  | 'for' identifier_list 'in' expr_list expr
  ;

while_block
  : 'while' expr expr
  ;

// == Declarations ==

arg_decl
  : ['const'] Identifier [':' type_expr] ['=' expr]
  ;

tuple_decl
  : '(' [arg_decl (',' arg_decl)*] ')'
  ;

var_decl
  : const_identifier_list ':' type_expr_list '=' expr_list
  | const_identifier_list '=' expr_list
  | const_identifier_list ':' type_expr_list
  ;

// == Expressions ==

macro_expr
  : '@' identifier_access '(' expr_list? ')'
  ;

typed_tuple_arg
  : ['const'] [Identifier ':'] type_expr
  ;

type_expr
  : '(' 'void'? ')'
  | '(' typed_tuple_arg ',' ')'
  | '(' typed_tuple_arg (',' typed_tuple_arg)+ ')'
  | 'fn' Identifier? tuple_decl type_expr?
  | type_expr ('[' (type_expr | Int | '...') ']')*
  | type_expr ('|' type_expr)+
  | identifier_access
  ;

assignment_expr
  : expr_list AssignmentOp expr_list
  ;

expr
  : logical_or_expr                // arithmetic
  | 'let' var_decl
  | logical_or_expr '..' '='? logical_or_expr [':' logical_or_expr] // range
  | struct_decl
  | func_decl
  | map_expr | array_expr | tuple_expr
  | if_block | while_block | for_block
  | '{' func_block* '}'
  ;

func_decl
  : tuple_decl type_expr? '=>' expr
  ;

struct_decl
  : tuple_decl '::' '{' file_block* '}'
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
  : postfix_expr '(' expr_list ')
  ;

postfix_expr
  : '(' expr ')'
  | '@' Identifier {'.' Identifier}
  | postfix_expr '[' expr ']'
  // i.e. array[1] or array[1:] or array[1::] or array[1:2:]
  // or array[] (lexical error) or array[:2:] ... and so on
  | postfix_expr '[' expr? [ ':' expr? [':' expr?] ] ']'
  | func_call
  | func_call '<|' postfix_expr
  | postfix_expr '|>' func_call
  | postfix_expr '.' Identifier
  | postfix_expr '++'
  | postfix_expr '--'
  | type_expr
  | Identifier
  | constant
  ;

unary_expr
  : postfix_expr
  | ('+' | '-' | '!' | '++' | '--') unary_expr
  ;

power_expr
  : unary_expr ('**' unary_expr)*

multiplicative_expr
  : power_expr (('*' | '/' | '%' | '%/') power_expr)*

additive_expr
  : multiplicative_expr (('+' | '-') multiplicative_expr)*
  ;

relational_expr
  : additive_expr (('<' | '>' | '>=' | '<=') additive_expr)*
  ;

equality_expr
  : relational_expr (('==' | '!=') relational_expr)*
  ;

logical_and_expr
  : equality_expr ('&&' equality_expr)*
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

const_identifier_list
  : ['const'] Identifier (',' ['const'] Identifier)*
  ;

type_expr_list
  : type_expr (',' type_expr)*
  ;

expr_list
  : expr (',' expr)*
  ;

expr_pair_lsit
  : expr ':' expr (',' expr ':' expr)*

var_decl_list
  : var_decl (',' var_decl)*
  ;
```
