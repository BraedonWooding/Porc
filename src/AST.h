#ifndef AST_H
#define AST_H

/*
    This is a gigantic file!
*/

// This is predominantly based upon the EBNF
// of course changes will occur
// i.e. ArgumentExpressionList => Vector(Expression)

#include <stdlib.h>
#include <stdio.h>

#include "vector.h"

// for type definitions
#define OPT(x) x

// more just till I lose my laziness to not add proper lines
#define NULL_RANGE (lineRange){0}

#define BASE_AST(x, fn_name, ...) x->parent.fn_name(x, ##__VA_ARGS__)
#define BASE_AST_OPT(x, fn_name, ...) x != NULL ? x->parent.fn_name(x, ##__VA_ARGS__) : true

typedef struct _line_range_t {
    size_t line_start;
    size_t line_end;
    size_t col_start;
    size_t col_end;
} lineRange;

typedef void(*fn_print)(void *self, FILE *io_out, int indent);
typedef void(*fn_free)(void *self);
typedef bool(*fn_validate)(void *self);

typedef struct _base_ast_t {
    lineRange position;
    fn_print print_tree;
    fn_free free_tree;
    fn_validate validate_tree;
} baseAST;

#pragma region AST_PRE_DECLARATIONS

typedef struct _file_level_expression_t *FileLevelExpression;
typedef struct _assignment_expression_t *AssignmentExpression;
typedef struct _top_level_expression_t *TopLevelExpression;
typedef struct _primary_expression_t *PrimaryExpression;
typedef struct _func_call_t *FuncCall;
typedef struct _postfix_expression_t *PostfixExpression;
typedef struct _unary_expression_t *UnaryExpression;
typedef struct _power_expression_t *PowerExpression;
typedef struct _multiplicative_expression_t *MultiplicativeExpression;
typedef struct _additive_expression_t *AdditiveExpression;
typedef struct _relational_expression_t *RelationalExpression;
typedef struct _equality_expression_t *EqualityExpression;
typedef struct _logical_and_expression_t *LogicalAndExpression;
typedef struct _logical_or_expression_t *LogicalOrExpression;
typedef struct _conditional_expression_t *ConditionalExpression;
typedef struct _block_t *Block;
typedef struct _expression_t *Expression;
typedef struct _compound_conditionals_t *CompoundConditionals;
typedef struct _while_loop_t *WhileLoop;
typedef struct _for_loop_t *ForLoop;
typedef struct _for_loop_contents_t *ForLoopContents;
typedef struct _else_block_t *ElseBlock;
typedef struct _if_block_t *IfBlock;
typedef struct _tuple_member_t *TupleMember;
typedef struct _tuple_definition_t *TupleDefinition;
typedef struct _type_expression_t *TypeExpression;
typedef struct _type_member_t *TypeMember;
typedef struct _func_definition_t *FuncDefinition;
typedef struct _constant_t *Constant;
typedef struct _array_constant_t *ArrayConstant;
typedef struct _map_constant_t *MapConstant;

#pragma endregion

// @NOTE: only some of the values will be valid
//        for each node, validate should be called
//        to verify that the types are valid
typedef enum TypeTagAST {
    AST_FILE_LEVEL_EXPRESSION,
    AST_ASSIGNMENT_EXPRESSION,
    AST_TOP_LEVEL_EXPRESSION,
    AST_PRIMARY_EXPRESSION,
    AST_FUNC_CALL,
    AST_POSTFIX_EXPRESSION,
    AST_UNARY_EXPRESSION,
    AST_POWER_EXPRESSION,
    AST_MULTIPLICATIVE_EXPRESSION,
    AST_ADDITIVE_EXPRESSION,
    AST_RELATIONAL_EXPRESSION,
    AST_EQUALITY_EXPRESSION,
    AST_LOGICAL_AND_EXPRESSION,
    AST_LOGICAL_OR_EXPRESSION,
    AST_CONDITIONAL_EXPRESSION,
    AST_BLOCK,
    AST_EXPRESSION,
    AST_COMPOUND_CONDITIONALS,
    AST_WHILE_LOOP,
    AST_FOR_LOOP,
    AST_FOR_LOOP_CONTENTS,
    AST_FOR_LOOP_TRADITIONAL,
    AST_FOR_LOOP_IN,
    AST_ELSE_BLOCK,
    AST_IF_BLOCK,
    AST_TUPLE_MEMBER,
    AST_TUPLE_DEFINITION,
    AST_TYPE_EXPRESSION,
    AST_TYPE_MEMBER,
    AST_FUNC_DEFINITION,
    AST_CONSTANT,
    AST_IDENTIFIER,
    AST_FUNC_CALL_L_FOLD, // a() <- b
    AST_FUNC_CALL_R_FOLD, // b -> a()
    AST_FUNC_DEFINITION_BLOCK,
    AST_TUPLE_DEFINITION_BLOCK,
    AST_POSTFIX_OP,
    AST_POSTFIX_MEMBER_ACCESS, // a.b
    AST_POSTFIX_INDEX, // a[b]
    AST_POSTFIX_SLICE, // a[b:c:d]
    AST_PREFIX_OP,
    AST_ARRAY,
    AST_MAP,
    AST_FLT,
    AST_INT,
    AST_STR,
    AST_CHAR,
} TypeTagAST;

typedef enum AssignmentOp {
    ASSIGN_OP_ADD_EQL,
    ASSIGN_OP_SUB_EQL,
    ASSIGN_OP_EQL,
    ASSIGN_OP_DIV_EQL,
    ASSIGN_OP_POW_EQL,
    ASSIGN_OP_MOD_EQL,
    ASSIGN_OP_MUL_EQL,
    ASSIGN_OP_INT_DIV_EQL,
} AssignmentOp;

typedef enum PostfixOp {
    POSTFIX_OP_INCR,
    POSTFIX_OP_DECR,
} PostfixOp;

typedef enum PrefixOp {
    PREFIX_OP_NEGATE,
    PREFIX_OP_NEGATIVE,
    PREFIX_OP_POSITIVE,
    PREFIX_OP_INCR,
    PREFIX_OP_DECR
} PrefixOp;

typedef enum MultiplicativeOp {
    MULTIPLICATIVE_OP_MUL,
    MULTIPLICATIVE_OP_DIV,
    MULTIPLICATIVE_OP_MOD,
    MULTIPLICATIVE_OP_INT_DIV,
} MultiplicativeOp;

typedef enum AdditiveOp {
    ADDITIVE_OP_SUB,
    ADDITIVE_OP_ADD,
} AdditiveOp;

typedef enum RelationalOp {
    RELATIONAL_OP_GT,
    RELATIONAL_OP_LT,
    RELATIONAL_OP_GTE,
    RELATIONAL_OP_LTE,
} RelationalOp;

typedef enum EqualityOp {
    EQUALITY_OP_EQ,
    EQUALITY_OP_NEQ,
} EqualityOp;

struct _file_level_expression_t {
    baseAST parent;
    Vector(TopLevelExpression) expressions;
};

FileLevelExpression ast_new_file_level_expr(lineRange pos, Vector(TopLevelExpression) exprs);

struct _top_level_expression_t {
    baseAST parent;
    union {
        AssignmentExpression assignment_expr;
        Expression expression;
    };
    TypeTagAST tag;
    bool ret; // 'return' expression
};

TopLevelExpression ast_new_top_level_expr(lineRange pos, void *arg, TypeTagAST tag, bool ret);

struct _primary_expression_t {
    baseAST parent;
    union {
        char *id;
        Expression expression;
        Constant constant;
        TypeExpression type_expr;
    };
    TypeTagAST tag;
};

PrimaryExpression ast_new_primary_expr(lineRange pos, void *arg, TypeTagAST tag);

struct _assignment_expression_t {
    baseAST parent;
    union {
        struct {
            TupleDefinition lhs;
            OPT(Expression) result;
        } declaration_with_type;
        struct {
            Expression lhs;
            AssignmentOp op;
            Expression rhs;
        } assignment;
    };
    // if true is declaration_with_type else assignment
    bool is_declaration;
};

AssignmentExpression ast_new_assign_expr_decl(lineRange pos, TupleDefinition lhs, OPT(Expression) res);
AssignmentExpression ast_new_assign_expr(lineRange pos, Expression lhs, AssignmentOp op, Expression rhs);

struct _func_call_t {
    baseAST parent;
    PostfixExpression func;
    Vector(Expression) args;
};

FuncCall ast_new_func_call(lineRange pos, PostfixExpression func, Vector(Expression) args);

struct _postfix_expression_t {
    baseAST parent;
    union {
        PrimaryExpression primary_expr;
        struct {
            PostfixExpression lhs;
            Expression index;
        } index_expr;
        struct {
            PostfixExpression lhs;
            OPT(Expression) start_index;
            OPT(Expression) end_index;
            OPT(Expression) step;
        } slice_expr;
        FuncCall func_call;
        // @NOTE: you can grab what kind of fold from
        // the type tag.
        // Maybe we should make the type tag 'kind' independent
        // and have a boolean field here to determine the kind?
        struct {
            FuncCall func;
            PostfixExpression to_fold;
        } fold_expr;
        struct {
            PostfixExpression lhs;
            PostfixOp op;
        } postfix_op;
        struct {
            PostfixExpression lhs;
            char *rhs;
        } member_access;
    };
    TypeTagAST tag;
};

PostfixExpression ast_new_postfix_expr_gen(lineRange pos, void *arg, TypeTagAST tag);
PostfixExpression ast_new_postfix_expr_fold(lineRange pos, FuncCall func, PostfixExpression to_fold, TypeTagAST tag);
PostfixExpression ast_new_postfix_expr_member(lineRange pos, PostfixExpression lhs, char *id, TypeTagAST tag);
PostfixExpression ast_new_postfix_expr_index(lineRange pos, PostfixExpression lhs, Expression index, TypeTagAST tag);
PostfixExpression ast_new_postfix_expr_slice(lineRange pos, PostfixExpression lhs, OPT(Expression) start, OPT(Expression) end, OPT(Expression) step, TypeTagAST tag);
PostfixExpression ast_new_postfix_expr_op(lineRange pos, PostfixExpression lhs, PostfixOp op, TypeTagAST tag);

struct _unary_expression_t {
    baseAST parent;
    union {
        PostfixExpression postfix_expr;
        struct {
            UnaryExpression rhs;
            PrefixOp op;
        } prefix_op;
    };
    TypeTagAST tag;
};

UnaryExpression ast_new_unary_expr(lineRange pos, PostfixExpression postfix_expr, TypeTagAST tag);
UnaryExpression ast_new_unary_expr_op(lineRange pos, UnaryExpression rhs, PrefixOp op, TypeTagAST tag);

struct _power_expression_t {
    baseAST parent;
    union {
        UnaryExpression unary_expr;
        struct {
            PowerExpression lhs;
            UnaryExpression rhs;
        } power_expr;
    };
    TypeTagAST tag;
};

PowerExpression ast_new_pow_expr_short(lineRange pos, UnaryExpression unary_expr, TypeTagAST tag);
PowerExpression ast_new_pow_expr(lineRange pos, PowerExpression lhs, UnaryExpression rhs, TypeTagAST tag);

struct _multiplicative_expression_t {
    baseAST parent;
    union {
        PowerExpression power_expr;
        struct {
            MultiplicativeExpression lhs;
            MultiplicativeOp op;
            PowerExpression rhs;
        } multiplicative_expr;
    };
    TypeTagAST tag;
};

MultiplicativeExpression ast_new_mul_expr_short(lineRange pos, PowerExpression power_expr, TypeTagAST tag);
MultiplicativeExpression ast_new_mul_expr(lineRange pos, MultiplicativeExpression lhs, MultiplicativeOp op, PowerExpression rhs, TypeTagAST tag);

struct _additive_expression_t {
    baseAST parent;
    union {
        MultiplicativeExpression multiplicative_expr;
        struct {
            AdditiveExpression lhs;
            AdditiveOp op;
            MultiplicativeExpression rhs;
        } additive_expr;
    };
    TypeTagAST tag;
};

AdditiveExpression ast_new_add_expr_short(lineRange pos, MultiplicativeExpression expr, TypeTagAST tag);
AdditiveExpression ast_new_add_expr(lineRange pos, AdditiveExpression lhs, AdditiveOp op, MultiplicativeExpression rhs, TypeTagAST tag);

struct _relational_expression_t {
    baseAST parent;
    union {
        AdditiveExpression additive_expr;
        struct {
            RelationalExpression lhs;
            RelationalOp op;
            AdditiveExpression rhs;
        } relational_expr;
    };
    TypeTagAST tag;
};

RelationalExpression ast_new_relational_expr_short(lineRange pos, AdditiveExpression expr, TypeTagAST tag);
RelationalExpression ast_new_relational_expr(lineRange pos, RelationalExpression lhs, RelationalOp op, AdditiveExpression rhs, TypeTagAST tag);

struct _equality_expression_t {
    baseAST parent;
    union {
        RelationalExpression relational_expr;
        struct {
            EqualityExpression lhs;
            EqualityOp op;
            RelationalExpression rhs;
        } equality_expr;
    };
    TypeTagAST tag;
};

EqualityExpression ast_new_eq_expr_short(lineRange pos, RelationalExpression expr, TypeTagAST tag);
EqualityExpression ast_new_eq_expr(lineRange pos, EqualityExpression lhs, EqualityOp op, RelationalExpression rhs, TypeTagAST tag);

struct _logical_and_expression_t {
    baseAST parent;
    union {
        EqualityExpression equality_expr;
        struct {
            LogicalAndExpression lhs;
            EqualityExpression rhs;
        } logical_and_expr;
    };
    TypeTagAST tag;
};

LogicalAndExpression ast_new_logic_and_expr_short(lineRange pos, EqualityExpression expr, TypeTagAST tag);
LogicalAndExpression ast_new_logic_and_expr(lineRange pos, LogicalAndExpression lhs, EqualityExpression rhs, TypeTagAST tag);

struct _logical_or_expression_t {
    baseAST parent;
    union {
        LogicalAndExpression logical_and_expr;
        struct {
            LogicalOrExpression lhs;
            LogicalAndExpression rhs;
        } logical_or_expr;
    };
    TypeTagAST tag;
};

LogicalOrExpression ast_new_logic_or_expr_short(lineRange pos, LogicalAndExpression expr, TypeTagAST tag);
LogicalOrExpression ast_new_logic_or_expr(lineRange pos, LogicalOrExpression lhs, LogicalAndExpression rhs, TypeTagAST tag);

struct _conditional_expression_t {
    baseAST parent;
    LogicalOrExpression logical_or_expr;
    struct {
        Expression if_true;
        Expression if_false;
    } ternary;
    bool is_ternary;
};

ConditionalExpression ast_new_conditional_expr(lineRange pos, LogicalOrExpression expr);
ConditionalExpression ast_new_conditional_expr_tern(lineRange pos, LogicalOrExpression expr, Expression if_expr, Expression else_expr);

struct _block_t {
    baseAST parent;
    Vector(TopLevelExpression) topLevelExpressions;
};

Block ast_new_block(lineRange pos, Vector(TopLevelExpression) topLevelExpressions);

struct _expression_t {
    baseAST parent;
    union {
        ConditionalExpression conditional_expr;
        struct {
            FuncDefinition func_def;
            Block block;
        } func_def_block;
        struct {
            TupleDefinition tuple_def;
            Block block;
        } tuple_def_block;
        CompoundConditionals compound_conditional;
    };
    TypeTagAST tag;
};

Expression ast_new_expr_gen(lineRange pos, void *arg, TypeTagAST tag);
Expression ast_new_expr_func_block(lineRange pos, FuncDefinition func_def, Block block, TypeTagAST tag);
Expression ast_new_expr_tuple_block(lineRange pos, TupleDefinition tuple_def, Block block, TypeTagAST tag);

struct _compound_conditionals_t {
    baseAST parent;
    union {
        ForLoop for_loop;
        WhileLoop while_loop;
        IfBlock if_block;
    };
    TypeTagAST tag;
};

CompoundConditionals ast_new_compound_conditional(lineRange pos, void *arg, TypeTagAST tag);

struct _while_loop_t {
    baseAST parent;
    ConditionalExpression conditional_expr;
    Block block;
    OPT(Block) else_block;
};

WhileLoop ast_new_while_loop(lineRange pos, ConditionalExpression cond_expr, Block block, OPT(Block) else_block);

struct _for_loop_t {
    baseAST parent;
    ForLoopContents contents;
    Block block;
    OPT(Block) else_block;
};

ForLoop ast_new_for_loop(lineRange pos, ForLoopContents contents, Block block, OPT(Block) else_block);

struct _for_loop_contents_t {
    baseAST parent;
    union {
        struct {
            // could have tuple_member here
            Vector(char*) ids;
            ConditionalExpression conditional_expr;
        } for_in;
        struct {
            OPT(AssignmentExpression) pre_expr;
            OPT(ConditionalExpression) condition_expr;
            OPT(ConditionalExpression) post_expr;
        } for_traditional;
    };
    TypeTagAST tag;
};

ForLoopContents ast_new_for_loop_contents_in(lineRange pos, Vector(char*) ids, ConditionalExpression cond_expr, TypeTagAST tag);
ForLoopContents ast_new_for_loop_contents_in(lineRange pos, Vector(char*) ids, ConditionalExpression cond_expr, TypeTagAST tag);

struct _if_block_t {
    baseAST parent;
    ConditionalExpression condition;
    Block block;
    OPT(ElseBlock) else_block;
};

IfBlock ast_new_if_block(lineRange pos, ConditionalExpression cond, Block block, OPT(Block) else_block);

struct _else_block_t {
    baseAST parent;
    union {
        IfBlock if_block;
        Block else_block;
    };
    TypeTagAST tag;
};

ElseBlock ast_new_else_block(lineRange pos, void *arg, TypeTagAST tag);

struct _tuple_member_t {
    baseAST parent;
    char *id;
    OPT(TypeExpression) type;
};

TupleMember ast_new_tuple_member(lineRange pos, char *id, OPT(TypeExpression) type);

struct _tuple_definition_t {
    baseAST parent;
    Vector(TupleMember) tuple_members;
};

TupleDefinition ast_new_tuple_def(lineRange pos, Vector(TupleMember) members);

struct _type_member_t {
    baseAST parent;
    union {
        FuncCall func_call;
        char *id;
        FuncDefinition func_def;
        OPT(TupleDefinition) tuple_def;
        struct {
            TypeExpression lhs;
            OPT(TypeExpression) rhs;
        } ArrayOrMap;
    };
    TypeTagAST tag;
};

TypeMember ast_new_type_member_gen(lineRange pos, void *arg, TypeTagAST tag);
TypeMember ast_new_type_member_array_map(lineRange pos, TypeExpression lhs, OPT(TypeExpression) rhs, TypeTagAST tag);

struct _type_expression_t {
    baseAST parent;
    TypeMember lhs;
    OPT(TypeExpression) or_type;
    OPT(TypeExpression) implements_type;
};

TypeExpression ast_new_type_expr(lineRange pos, TypeMember lhs, OPT(TypeExpression) or_type, OPT(TypeExpression) impl_type);

struct _func_definition_t {
    baseAST parent;
    OPT(TupleDefinition) arg_defs;
    OPT(TypeExpression) ret_type;
};

FuncDefinition ast_new_func_def(lineRange pos, OPT(TupleDefinition) def, OPT(TypeExpression) ret_type);

struct _constant_t {
    baseAST parent;
    union {
        ArrayConstant array;
        MapConstant map;
        double flt;
        long long int integer;
        char *str;
        char character;
    };
    TypeTagAST tag;
};

Constant ast_new_constant(lineRange pos, void *arg, TypeTagAST tag);
#define ast_new_constant_lit(pos, arg, tag) ast_new_constant(pos, &(typeof(arg){arg}), tag)

struct _map_constant_t {
    baseAST parent;
    // we could just have a hashtable for this????
    Vector(Expression) keys;
    Vector(Expression) values;
};

MapConstant ast_new_map_constant(lineRange pos, Vector(Expression) keys, Vector(Expression) values);

struct _array_constant_t {
    baseAST parent;
    Vector(Expression) contents;
};

ArrayConstant ast_new_array_constant(lineRange pos, Vector(Expression) contents);

#endif