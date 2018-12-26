#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"
#include "ast.h"
#include "err.h"
#include "option.h"

typedef struct _parser_state_t {
    Tokenizer tok;
} *ParserState;

ParserState parser_new(Tokenizer tok);
Result(FileLevelExpression) parse_file_level_expr(ParserState state);
Result(TopLevelExpression) parse_top_level_expr(ParserState state);
Result(PrimaryExpression) parser_primary_expr(ParserState state);
Result(AssignmentExpression) parser_assignment_expr(ParserState state);
Result(FuncCall) parser_func_call(ParserState state);
Result(PostfixExpression) parser_postfix_expr(ParserState state);
Result(UnaryExpression) parser_unary_expr(ParserState state);
Result(PowerExpression) parser_prefix_expr(ParserState state);
Result(MultiplicativeExpression) parse_multiplicative_expr(ParserState state);
Result(AdditiveExpression) parse_additive_expr(ParserState state);
Result(RelationalExpression) parse_relational_expr(ParserState state);
Result(EqualityExpression) parse_equality_expr(ParserState state);
Result(LogicalAndExpression) parse_logical_and_expr(ParserState state);
Result(LogicalOrExpression) parse_logical_or_expr(ParserState state);
Result(ConditionalExpression) parse_conditional_expr(ParserState state);
Result(Block) parse_block_expr(ParserState state);
Result(Expression) parse_expr(ParserState state);
Result(CompoundConditionals) parse_compound_conditionals(ParserState state);
Result(WhileLoop) parse_while_loop(ParserState state);
Result(ForLoop) parse_for_loop(ParserState state);
Result(ForLoopContents) parse_for_loop_contents(ParserState state);
Result(ElseBlock) parse_else_block(ParserState state);
Result(IfBlock) parse_if_block(ParserState state);
Result(TupleMember) parse_tuple_member(ParserState state);
Result(TupleDefinition) parse_tuple_def(ParserState state);
Result(TypeExpression) parse_type_expr(ParserState state);
Result(TypeMember) parse_type_member(ParserState state);
Result(FuncDefinition) parse_func_def(ParserState state);
Result(Constant) parse_constant(ParserState state);
Result(ArrayConstant) parse_array_constant(ParserState state);
Result(MapConstant) parse_map_constant(ParserState state);

#endif