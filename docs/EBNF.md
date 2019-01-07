# EBNF

## TODO

Small todo section

- `where`
- `block` on while/for can be empty

## Actual grammar

This is more to make sure we something to make sure our parser is consistent.

A few notes:

- `[a]` is identical to `(a)?` (i.e. 0 or 1)
- `(a)+` means `a` repeated (1+)
- `(a)*` means `a` repeated or not (0+)
  - Basically identical to `[(a)+]` (which is `((a)+)?`)
- Semicolons are kinda implemented incorrectly here, basically the rules follow the 'expression language' return rules followed by languages like rust i.e. `return` returns but also the last statement which doesn't have a `;` (note this works inside if statements)
  - i.e. `if (x) 1 else 2` is identical to `if (x) return 1 else return 2;` and `return if (x) 1 else 2;`

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

// @Q: do we want to allow `0.`
FloatDecimal ::= DigitSeq? '.' DigitSeq
              | DigitSeq '.'

FloatExponent ::= ('e' | 'E') [Sign] DigitSeq

// Operators

AssignmentOp ::= '=' | '*=' | '/=' | '**=' | '%/=' | '%=' | '+=' | '-='

// Main Expressions

fileLevelExpression
    : (topLevelExpression ';'?)*
    ;

topLevelExpression
    : assignmentExpression
    | expression
    | 'return' expression
    ;

primaryExpression
    : '(' expression ')'
    | Identifier
    | constant
    | type_expression
    ;

assignmentExpression
    : tuple_definition ['=' expression]
    | expression AssignmentOp expression
    ;

/*
    1 + a -> b() + 1 ==== 1 + b(a) + 1
    1 + a() <- b() <- c + 1 ==== 1 + a(b(c)) + 1
*/

func_call
    : postfixExpression '(' argumentExpressionList? ')'
    ;

// @Q: I'm like 99% sure these bindings are correct (consult the above diagram)
// However they could still be wrong if I'm considering a case incorrectly :).
postfixExpression
    : primaryExpression
    | '@' Identifier {'.' Identifier}
    | postfixExpression '[' expression ']'
    // i.e. array[1] or array[1:] or array[1::] or array[1:2:]
    // or array[] (lexical error) or array[:2:] ... and so on
    | postfixExpression '[' expression? [ ':' expression? [':' expression?] ] ']'
    | func_call
    | func_call '<-' postfixExpression
    | postfixExpression '->' func_call
    | postfixExpression '.' Identifier
    | postfixExpression '++'
    | postfixExpression '--'
    ;

argumentExpressionList
    : expression
    | argumentExpressionList ',' expression
    ;

unaryExpression
    : postfixExpression
    | ('+' | '-' | '!' | '++' | '--') unaryExpression
    ;

powerExpression
    : unaryExpression
    | powerExpression '**' unaryExpression

multiplicativeExpression
    : powerExpression
    | multiplicativeExpression ('*' | '/' | '%' | '%/') powerExpression

additiveExpression
    : multiplicativeExpression
    | additiveExpression ('+' | '-') multiplicativeExpression
    ;

relationalExpression
    : additiveExpression
    | relationalExpression ('<' | '>' | '>=' | '<=') additiveExpression
    ;

equalityExpression
    : relationalExpression
    | equalityExpression ('==' | '!=') relationalExpression
    ;

logicalAndExpression
    : equalityExpression
    | logicalAndExpression '&&' equalityExpression
    ;

logicalOrExpression
    : logicalAndExpression
    | logicalOrExpression '||' logicalAndExpression

conditionalExpression
    // ternary support as well
    : logicalOrExpression ['?' expression ':' expression]
    ;

block
    : '{' (topLevelExpression ';'?)* '}'
    | topLevelExpression
    ;

expression
    : conditionalExpression
    | func_definition '=>' block
    | tuple_definition '::' block
    | compound_conditionals
    ;

compound_conditionals
    : for_loop
    | while_loop
    | if_block
    ;

while_loop
    : 'while' '(' conditionalExpression ')' block ['else' block]
    | 'while' conditionalExpression block ['else' block]
    ;

for_loop_contents
    : Identifier [',' Identifier] 'in' conditionalExpression
    | assignmentExpression? ';' conditionalExpression? ';' conditionalExpression?
    ;

for_loop
    : 'for' '(' for_loop_contents ')' block ['else' block]
    | 'for' for_loop_contents block ['else' block]
    ;

else_block
    : 'else' if_block
    | 'else' block
    ;

if_block
    : 'if' '(' conditionalExpression ')' block else_block?
    | 'if' conditionalExpression block else_block?
    ;

tuple_member
    : Identifier [ ':' type_expression ]
    ;

tuple_definition
    : tuple_member (',' tuple_member)*
    ;

type_member
    : func_call
    | Identifier
    | func_definition
    | '(' tuple_definition? ')'
    // array / map
    | '[' type_expression [ ':' type_expression ] ']'
    ;

type_expression
    : type_member
    | type_expression ['|' type_member] ['^' type_member]
    ;

func_definition
    : 'fn' '(' tuple_definition? ')' type_expression?

constant
    : array_constant
    | map_constant
    | Float
    | Int
    | String
    | Char

array_constant: '[' expression (',' expression)* ']'
map_constant: '[' expression ':' expression (',' expression ':' expression)* ']'
```
