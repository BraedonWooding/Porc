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

- Is `x..y:z |> f()` parsed correclty as `f(x..y:z)`? and same for `f() <| x..y:z`
  - Yes I believe so!  However a bug exists due to the semicolons see range.porc seems to only be with ranges
- Folding expressions `|>` and `<|` are parsed incorrectly I think??
  - I think I want `g() <| x + 2 == 4 <| f()` parsed as `g(x + 2) == f(4)`
  - It should work now
- Bitwise operations with proper associativity and precedence

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

file_decl
  : (func_block | type_decl)*
  ;

type_decl
  // the semicolon is purely optional if you include the block
  : 'type' identifier 'is' type_expr ['{' struct_block* '}'] ';'?
  | 'type' identifier '{' struct_block* '}' ';'?
  ;

struct_block
  // identifier access has lower priority to var_decl
  : identifier_access? var_decl ';'?
  | macro_expr ';'
  | type_decl
  ;

func_block
  : assignment_expr ';'?
  | var_decl ';'?
  // you require a semicolon for all expressions that aren't just a block
  // for example x := if (y) 2 else 4;
  // you don't require them for any function definitions (unless you call them)
  // note this also applies to folding expressions if you use any folding you
  // have to end it with `;` i.e. A :: () => { } |> ((a) => a())();
  | ['yield'] ['return' | 'continue' | '=' | 'break'] expr ';'?
  ;

if_block
  // you can elide the `;` for if statements that return values (i.e. no block just expr)
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

tuple_value
  : Identifier [':' type_expr] ['=' expr]
  ;

tuple_type
  : [ Identifier ':' ] type_expr
  ;

tuple_value_decl
  : '(' [tuple_value (',' tuple_value)*] ')'
  ;

tuple_type_decl
  : '(' [tuple_type (',' tuple_type)*] ')'
  ;

var_decl
  : identifier_list ':' type_expr_list (':' | '=') expr_list
  | identifier_list ':' type_expr_list
  | identifier_list ('::' | ':=') expr_list
  ;

// == Expressions ==

macro_expr
  : '@' identifier_access '(' expr_list? ')'
  ;

type_expr
  : tuple_type_decl
  | tuple_type_decl '->' type_expr
  | '$' identifier
  | identifier_access '[' type_expr (',' type_expr)+ ']'
  | identifier_access
  | type_expr ('|' type_expr)+
  ;

assignment_expr
  : expr_list AssignmentOp expr_list
  ;

expr
  : logical_or_expr                // arithmetic
  | 'let' (var_decl | assignment_expr)
  | lambda_decl
  | array_expr | tuple_expr
  | if_block | while_block | for_block
  | '{' func_block* '}'
  ;

lambda_decl
  : tuple_value_decl ['->' type_expr] '=>' '{' func_block '}'
  ;

tuple_expr
  : '(' expr_list? ')'
  ;

array_expr
  : '[' expr_list ']'
  ;

func_call
  : atom_expr '(' expr_list ')
  ;

atom
  : '(' expr ')'
  | '@' identifier_access '(' expr_list ')'
  // @POSSIBLE_FIX: fix folds not having a nice associativity
  //       I think this is the fix that we want (moved from expr to additive)
  //       But I will need to think abou it!
  | additive_expr '|>' atom
  | atom '<|' additive_expr
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

// NOTE: this is slightly wrong
//       range can only be applied to a single additive expr
//       or rather it itself isn't really one... but it has to be
//       so its a weird mix?  Basically it can't include itself
additive_expr
  : multiplicative_expr (('+' | '-') multiplicative_expr)*
  | additive_expr? '..' '='? additive_expr? [':' additive_expr]
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
```
