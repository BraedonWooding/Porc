#include "ast.hpp"

// #define INDENT_STR "    "

// // typedef void(*fn_print)(void *self, FILE *io_out, int indent);
// // typedef void(*fn_free)(void *self);
// // typedef bool(*fn_validate)(void *self);

// void write_indent(FILE *fp, int indent) {
//     for (int i = 0; i < indent; i++) fputs(INDENT_STR, fp);
// }

// #define AST_TAG_ERR(expr) fprintf(stderr, \
//     __FILE__ ":%d COMPILER INTERNAL ERROR (compiler bug): Invalid Tag (%d) exiting...\n", \
//     __LINE__, expr->tag)

// #define VALIDATION_ERR(err, ...) fprintf(stderr, "Compiler AST Validation Failed (compiler bug): " err "\n", ##__VA_ARGS__)

// #pragma region FileLevelExpression

// static void file_level_expr_print(void *self, FILE *io_out, int indent) {
//     FileLevelExpression expr = self;
//     write_indent(io_out, indent);
//     fprintf(io_out, "FileLevelExpr(\n");
//     vec_foreach(expr->expressions, TopLevelExpression top_level, {
//         BASE_AST(top_level, print_tree, io_out, indent + 1);
//     });
//     write_indent(io_out, indent);
//     fprintf(io_out, ")\n");
// }

// static void file_level_expr_free(void *self) {
//     FileLevelExpression expr = self;
//     vec_foreach(expr->expressions, TopLevelExpression top_level, {
//         BASE_AST(top_level, free_tree);
//     })
//     vec_free(expr->expressions);
//     free(expr);
// }

// static bool file_level_expr_validate(void *self) {
//     FileLevelExpression expr = self;
//     bool ret = expr->expressions != NULL;
//     if (!ret) {
//         VALIDATION_ERR("Vector(Expression) is null which is undefined");
//         return ret;
//     }

//     vec_foreach(expr->expressions, TopLevelExpression top_level, {
//         // &= to make sure that a single error makes this return false
//         // but we don't want to short circuit and we want to get as many as we can
//         ret &= BASE_AST(top_level, validate_tree);
//     })

//     return ret;
// }

// FileLevelExpression ast_new_file_level_expr(lineRange pos, Vector(TopLevelExpression) exprs) {
//     FileLevelExpression expr = malloc(sizeof(struct _file_level_expression_t));
//     expr->expressions = exprs;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = file_level_expr_print,
//         .free_tree = file_level_expr_free,
//         .validate_tree = file_level_expr_validate
//     };
//     return expr;
// }

// #pragma endregion

// #pragma region TopLevelExpression

// static void top_level_expr_print(void *self, FILE *io_out, int indent) {
//     TopLevelExpression expr = self;
//     write_indent(io_out, indent);
//     fprintf(io_out, "TopLevelExpr(\n");
//     switch (expr->tag) {
//         case AST_EXPRESSION: {
//             BASE_AST(expr->expression, print_tree, io_out, indent + 1);
//         } break;
//         case AST_ASSIGNMENT_EXPRESSION: {
//             BASE_AST(expr->assignment_expr, print_tree, io_out, indent + 1);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         } break;
//     }
//     write_indent(io_out, indent);
//     fprintf(io_out, ")\n");
// }

// static void top_level_expr_free(void *self) {
//     TopLevelExpression expr = self;
//     switch (expr->tag) {
//         case AST_EXPRESSION: {
//             BASE_AST(expr->expression, free_tree);
//         } break;
//         case AST_ASSIGNMENT_EXPRESSION: {
//             BASE_AST(expr->assignment_expr, free_tree);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         } break;
//     }
//     free(expr);
// }

// static bool top_level_expr_validate(void *self) {
//     TopLevelExpression expr = self;
//     bool ret = true;
//     switch (expr->tag) {
//         case AST_EXPRESSION: {
//             ret = BASE_AST(expr->expression, validate_tree);
//         } break;
//         case AST_ASSIGNMENT_EXPRESSION: {
//             ret = BASE_AST(expr->assignment_expr, validate_tree);

//         } break;
//         default: {
//             ret = false;
//             VALIDATION_ERR("Invalid tag (%d)", expr->tag);
//         } break;
//     }
//     return ret;
// }

// TopLevelExpression ast_new_top_level_expr(lineRange pos, void *arg, TypeTagAST tag, bool ret) {
//     TopLevelExpression expr = malloc(sizeof(struct _top_level_expression_t));
//     expr->ret = ret;
//     expr->tag = tag;
//     switch (tag) {
//         case AST_EXPRESSION: {
//             expr->expression = arg;
//         } break;
//         case AST_ASSIGNMENT_EXPRESSION: {
//             expr->assignment_expr = arg;
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         } break;
//     }
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = top_level_expr_print,
//         .free_tree = top_level_expr_free,
//         .validate_tree = top_level_expr_validate
//     };

//     return expr;
// }

// #pragma endregion

// #pragma region PrimaryExpression

// static void primary_expr_print(void *self, FILE *io_out, int indent) {
//     PrimaryExpression expr = self;
//     write_indent(io_out, indent);
//     fprintf(io_out, "PrimaryExpr(\n");
//     switch (expr->tag) {
//         case AST_EXPRESSION: {
//             BASE_AST(expr->expression, print_tree, io_out, indent + 1);
//         } break;
//         case AST_CONSTANT: {
//             BASE_AST(expr->constant, print_tree, io_out, indent + 1);
//         } break;
//         case AST_IDENTIFIER: {
//             write_indent(io_out, indent + 1);
//             fprintf(io_out, "Identifier(\"%s\")\n", expr->id);
//         } break;
//         case AST_TYPE_EXPRESSION: {
//             BASE_AST(expr->type_expr, print_tree, io_out, indent + 1);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         } break;
//     }
//     write_indent(io_out, indent);
//     fprintf(io_out, ")\n");
// }

// static void primary_expr_free(void *self) {
//     PrimaryExpression expr = self;
//     switch (expr->tag) {
//         case AST_EXPRESSION: {
//             BASE_AST(expr->expression, free_tree);
//         } break;
//         case AST_CONSTANT: {
//             BASE_AST(expr->constant, free_tree);
//         } break;
//         case AST_IDENTIFIER: {
//             free(expr->id);
//         } break;
//         case AST_TYPE_EXPRESSION: {
//             BASE_AST(expr->type_expr, free_tree);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         } break;
//     }
//     free(expr);
// }

// static bool primary_expr_validate(void *self) {
//     PrimaryExpression expr = self;
//     bool ret = true;
//     switch (expr->tag) {
//         case AST_EXPRESSION: {
//             ret &= BASE_AST(expr->expression, validate_tree);
//         } break;
//         case AST_CONSTANT: {
//             ret &= BASE_AST(expr->constant, validate_tree);
//         } break;
//         case AST_IDENTIFIER: {
//             ret &= expr->id != NULL;
//         } break;
//         case AST_TYPE_EXPRESSION: {
//             ret &= BASE_AST(expr->type_expr, validate_tree);
//         } break;
//         default: {
//             ret = false;
//             VALIDATION_ERR("Invalid tag (%d)", expr->tag);
//         } break;
//     }
//     return ret;
// }

// PrimaryExpression ast_new_primary_expr(lineRange pos, void *arg, TypeTagAST tag) {
//     PrimaryExpression expr = malloc(sizeof(struct _primary_expression_t));
//     expr->tag = tag;
//     switch (tag) {
//         case AST_EXPRESSION: {
//             expr->expression = arg;
//         } break;
//         case AST_CONSTANT: {
//             expr->constant = arg;
//         } break;
//         case AST_IDENTIFIER: {
//             expr->id = arg;
//         } break;
//         case AST_TYPE_EXPRESSION: {
//             expr->type_expr = arg;
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         } break;
//     }
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = top_level_expr_print,
//         .free_tree = top_level_expr_free,
//         .validate_tree = top_level_expr_validate
//     };

//     return expr;
// }

// #pragma endregion

// #pragma region AssignmentExpression

// static void assignment_expr_free(void *arg) {
//     AssignmentExpression expr = arg;
//     if (expr->is_declaration) {
//         BASE_AST(expr->declaration_with_type.lhs, free_tree);
//         BASE_AST_OPT(expr->declaration_with_type.result, free_tree);
//     } else {
//         BASE_AST(expr->assignment.lhs, free_tree);
//         BASE_AST(expr->assignment.rhs, free_tree);
//     }
//     free(expr);
// }

// static bool assignment_expr_validate(void *arg) {
//     AssignmentExpression expr = arg;
//     bool ret = true;
//     if (expr->is_declaration) {
//         ret &= BASE_AST(expr->declaration_with_type.lhs, validate_tree);
//         ret &= BASE_AST_OPT(expr->declaration_with_type.result, validate_tree);
//     } else {
//         ret &= BASE_AST(expr->assignment.lhs, validate_tree);
//         ret &= BASE_AST(expr->assignment.rhs, validate_tree);
//     }
//     return ret;
// }

// static void assignment_expr_print(void *arg, FILE *io_out, int indent) {
//     AssignmentExpression expr = arg;
//     write_indent(io_out, indent);
//     fprintf(io_out, "AssignmentExpr(\n");

//     if (expr->is_declaration) {
//         BASE_AST(expr->declaration_with_type.lhs, print_tree, io_out, indent + 1);
//         if (expr->declaration_with_type.result != NULL) {
//             fprintf(io_out, ",\n");
//             BASE_AST(expr->declaration_with_type.result, print_tree, io_out, indent + 1);
//         }
//     } else {
//         BASE_AST(expr->assignment.lhs, print_tree, io_out, indent + 1);
//         fprintf(io_out, ", op=%d,\n", expr->assignment.op);
//         BASE_AST(expr->assignment.rhs, print_tree, io_out, indent + 1);
//     }
// }

// AssignmentExpression ast_new_assign_expr_decl(lineRange pos, TupleDefinition lhs, OPT(Expression) res) {
//     AssignmentExpression expr = malloc(sizeof(struct _assignment_expression_t));
//     expr->declaration_with_type.lhs = lhs;
//     expr->declaration_with_type.result = res;
//     expr->is_declaration = true;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .free_tree = assignment_expr_free,
//         .print_tree = assignment_expr_print,
//         .validate_tree = assignment_expr_validate,
//     };
//     return expr;
// }

// AssignmentExpression ast_new_assign_expr(lineRange pos, Expression lhs, AssignmentOp op, Expression rhs) {
//     AssignmentExpression expr = malloc(sizeof(struct _assignment_expression_t));
//     expr->assignment.lhs = lhs;
//     expr->assignment.op = op;
//     expr->assignment.rhs = rhs;
//     expr->is_declaration = false;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .free_tree = assignment_expr_free,
//         .print_tree = assignment_expr_print,
//         .validate_tree = assignment_expr_validate,
//     };
//     return expr;
// }

// #pragma endregion

// #pragma region FuncCall

// static void func_call_free(void *self) {
//     FuncCall func = self;
//     vec_foreach(func->args, Expression expr, {
//         BASE_AST(expr, free_tree);
//     });
//     BASE_AST(func->func, free_tree);
//     free(func);
// }

// static bool func_call_validate(void *self) {
//     FuncCall func = self;
//     bool ret = true;

//     if (func->args != NULL) {
//         vec_foreach(func->args, Expression expr, {
//             ret &= BASE_AST(expr, validate_tree);
//         });
//     } else {
//         ret = false;
//     }
//     ret &= BASE_AST(func->func, validate_tree);
//     return ret;
// }

// static void func_call_print(void *self, FILE *io_out, int indent) {
//     FuncCall call = self;
//     write_indent(io_out, indent);
//     fprintf(io_out, "FuncCall(\n");
//     BASE_AST(call->func, print_tree, io_out, indent + 1);
//     write_indent(io_out, indent + 1);
//     fprintf(io_out, "args=\n");
//     vec_foreach(call->args, Expression expr, {
//         BASE_AST(expr, print_tree, io_out, indent + 1);
//     });
//     write_indent(io_out, indent);
//     fprintf(io_out, ")\n");
// }

// FuncCall ast_new_func_call(lineRange pos, PostfixExpression func, Vector(Expression) args) {
//     FuncCall call = malloc(sizeof(struct _func_call_t));
//     call->args = args;
//     call->func = func;
//     call->parent = (baseAST) {
//         .position = pos,
//         .free_tree = func_call_free,
//         .print_tree = func_call_print,
//         .validate_tree = func_call_validate,
//     };
//     return call;
// }

// #pragma endregion

// #pragma region PostfixExpression

// static void postfix_expr_print(void *self, FILE *io_out, int indent) {
//     PostfixExpression expr = self;
//     write_indent(io_out, indent);
//     fprintf(io_out, "PostfixExpr(\n");
//     switch (expr->tag) {
//         case AST_PRIMARY_EXPRESSION: {
//             BASE_AST(expr->primary_expr, print_tree, io_out, indent + 1);
//         } break;
//         case AST_POSTFIX_INDEX: {
//             BASE_AST(expr->index_expr.index, print_tree, io_out, indent + 1);
//             BASE_AST(expr->index_expr.lhs, print_tree, io_out, indent + 1);
//         } break;
//         case AST_POSTFIX_SLICE: {
//             BASE_AST(expr->slice_expr.lhs, print_tree, io_out, indent + 1);
//             BASE_AST_OPT(expr->slice_expr.start_index, print_tree, io_out, indent + 1);
//             BASE_AST_OPT(expr->slice_expr.end_index, print_tree, io_out, indent + 1);
//             BASE_AST_OPT(expr->slice_expr.end_index, print_tree, io_out, indent + 1);
//         } break;
//         case AST_FUNC_CALL: {
//             BASE_AST(expr->func_call, print_tree, io_out, indent + 1);
//         } break;
//         case AST_FUNC_CALL_L_FOLD: case AST_FUNC_CALL_R_FOLD: {
//             BASE_AST(expr->fold_expr.func, print_tree, io_out, indent + 1);
//             BASE_AST(expr->fold_expr.to_fold, print_tree, io_out, indent + 1);
//         } break;
//         case AST_POSTFIX_OP: {
//             BASE_AST(expr->postfix_op.lhs, print_tree, io_out, indent + 1);
//         } break;
//         case AST_POSTFIX_MEMBER_ACCESS: {
//             BASE_AST(expr->member_access.lhs, print_tree, io_out, indent + 1);
//             free(expr->member_access.rhs);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     write_indent(io_out, indent);
//     fprintf(io_out, ")\n");
// }

// static bool postfix_expr_validate(void *self) {
//     PostfixExpression expr = self;
//     bool ret = true;
//     switch (expr->tag) {
//         case AST_PRIMARY_EXPRESSION: {
//             ret &= BASE_AST(expr->primary_expr, validate_tree);
//         } break;
//         case AST_POSTFIX_INDEX: {
//             ret &= BASE_AST(expr->index_expr.index, validate_tree);
//             ret &= BASE_AST(expr->index_expr.lhs, validate_tree);
//         } break;
//         case AST_POSTFIX_SLICE: {
//             ret &= BASE_AST(expr->slice_expr.lhs, validate_tree);
//             ret &= BASE_AST_OPT(expr->slice_expr.start_index, validate_tree);
//             ret &= BASE_AST_OPT(expr->slice_expr.end_index, validate_tree);
//             ret &= BASE_AST_OPT(expr->slice_expr.end_index, validate_tree);
//         } break;
//         case AST_FUNC_CALL: {
//             ret &= BASE_AST(expr->func_call, validate_tree);
//         } break;
//         case AST_FUNC_CALL_L_FOLD: case AST_FUNC_CALL_R_FOLD: {
//             ret &= BASE_AST(expr->fold_expr.func, validate_tree);
//             ret &= BASE_AST(expr->fold_expr.to_fold, validate_tree);
//         } break;
//         case AST_POSTFIX_OP: {
//             ret &= BASE_AST(expr->postfix_op.lhs, validate_tree);
//         } break;
//         case AST_POSTFIX_MEMBER_ACCESS: {
//             ret &= BASE_AST(expr->member_access.lhs, validate_tree);
//             ret &= expr->member_access.rhs != NULL;
//         } break;
//         default: {
//             ret = false;
//             VALIDATION_ERR("Invalid tag (%d)", expr->tag);
//         }
//     }
//     return ret;
// }

// static void postfix_expr_free(void *self) {
//     PostfixExpression expr = self;
//     switch (expr->tag) {
//         case AST_PRIMARY_EXPRESSION: {
//             BASE_AST(expr->primary_expr, free_tree);
//         } break;
//         case AST_POSTFIX_INDEX: {
//             BASE_AST(expr->index_expr.index, free_tree);
//             BASE_AST(expr->index_expr.lhs, free_tree);
//         } break;
//         case AST_POSTFIX_SLICE: {
//             BASE_AST(expr->slice_expr.lhs, free_tree);
//             BASE_AST_OPT(expr->slice_expr.start_index, free_tree);
//             BASE_AST_OPT(expr->slice_expr.end_index, free_tree);
//             BASE_AST_OPT(expr->slice_expr.end_index, free_tree);
//         } break;
//         case AST_FUNC_CALL: {
//             BASE_AST(expr->func_call, free_tree);
//         } break;
//         case AST_FUNC_CALL_L_FOLD: case AST_FUNC_CALL_R_FOLD: {
//             BASE_AST(expr->fold_expr.func, free_tree);
//             BASE_AST(expr->fold_expr.to_fold, free_tree);
//         } break;
//         case AST_POSTFIX_OP: {
//             BASE_AST(expr->postfix_op.lhs, free_tree);
//         } break;
//         case AST_POSTFIX_MEMBER_ACCESS: {
//             BASE_AST(expr->member_access.lhs, free_tree);
//             free(expr->member_access.rhs);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     free(expr);
// }

// PostfixExpression ast_new_postfix_expr_gen(lineRange pos, void *arg, TypeTagAST tag) {
//     PostfixExpression expr = malloc(sizeof(struct _postfix_expression_t));
//     expr->tag = tag;
//     switch (tag) {
//         case AST_PRIMARY_EXPRESSION: {
//             expr->primary_expr = arg;
//         } break;
//         case AST_FUNC_CALL: {
//             expr->func_call = arg;
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         } break;
//     }
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = postfix_expr_print,
//         .free_tree = postfix_expr_free,
//         .validate_tree = postfix_expr_validate
//     };
//     return expr;
// }

// PostfixExpression ast_new_postfix_expr_fold(lineRange pos, FuncCall func, PostfixExpression to_fold, TypeTagAST tag) {
//     PostfixExpression expr = malloc(sizeof(struct _postfix_expression_t));
//     expr->tag = tag;
//     if (tag != AST_FUNC_CALL_L_FOLD && tag != AST_FUNC_CALL_R_FOLD) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->fold_expr.func = func;
//     expr->fold_expr.to_fold = to_fold;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = postfix_expr_print,
//         .free_tree = postfix_expr_free,
//         .validate_tree = postfix_expr_validate
//     };
//     return expr;
// }

// PostfixExpression ast_new_postfix_expr_member(lineRange pos, PostfixExpression lhs, char *id, TypeTagAST tag) {
//     PostfixExpression expr = malloc(sizeof(struct _postfix_expression_t));
//     expr->tag = tag;
//     if (tag != AST_POSTFIX_MEMBER_ACCESS) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->member_access.lhs = lhs;
//     expr->member_access.rhs = id;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = postfix_expr_print,
//         .free_tree = postfix_expr_free,
//         .validate_tree = postfix_expr_validate
//     };
//     return expr;
// }

// PostfixExpression ast_new_postfix_expr_index(lineRange pos, PostfixExpression lhs, Expression index, TypeTagAST tag) {
//     PostfixExpression expr = malloc(sizeof(struct _postfix_expression_t));
//     expr->tag = tag;
//     if (tag != AST_POSTFIX_INDEX) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->index_expr.lhs = lhs;
//     expr->index_expr.index = index;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = postfix_expr_print,
//         .free_tree = postfix_expr_free,
//         .validate_tree = postfix_expr_validate
//     };
//     return expr;
// }

// PostfixExpression ast_new_postfix_expr_slice(lineRange pos, PostfixExpression lhs, OPT(Expression) start, OPT(Expression) end, OPT(Expression) step, TypeTagAST tag) {
//     PostfixExpression expr = malloc(sizeof(struct _postfix_expression_t));
//     expr->tag = tag;
//     if (tag != AST_POSTFIX_SLICE) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->slice_expr.lhs = lhs;
//     expr->slice_expr.start_index = start;
//     expr->slice_expr.end_index = end;
//     expr->slice_expr.step = step;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = postfix_expr_print,
//         .free_tree = postfix_expr_free,
//         .validate_tree = postfix_expr_validate
//     };
//     return expr;
// }

// PostfixExpression ast_new_postfix_expr_op(lineRange pos, PostfixExpression lhs, PostfixOp op, TypeTagAST tag) {
//     PostfixExpression expr = malloc(sizeof(struct _postfix_expression_t));
//     expr->tag = tag;
//     if (tag != AST_POSTFIX_OP) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->postfix_op.lhs = lhs;
//     expr->postfix_op.op = op;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = postfix_expr_print,
//         .free_tree = postfix_expr_free,
//         .validate_tree = postfix_expr_validate
//     };
//     return expr;
// }

// #pragma endregion

// #pragma region UnaryExpression

// static void unary_expr_print(void *self, FILE *io_out, int indent) {
//     UnaryExpression expr = self;
//     write_indent(io_out, indent);
//     fprintf(io_out, "UnaryExpr(\n");
//     switch (expr->tag) {
//         case AST_POSTFIX_EXPRESSION: {
//             BASE_AST(expr->postfix_expr, print_tree, io_out, indent + 1);
//         } break;
//         case AST_PREFIX_OP: {
//             BASE_AST(expr->prefix_op.rhs, print_tree, io_out, indent + 1);
//             write_indent(io_out, indent + 1);
//             fprintf(io_out, "op=%d\n", expr->prefix_op.op);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     write_indent(io_out, indent);
//     fprintf(io_out, ")\n");
// }

// static bool unary_expr_validate(void *self) {
//     UnaryExpression expr = self;
//     bool ret = true;
//     switch (expr->tag) {
//         case AST_POSTFIX_EXPRESSION: {
//             ret &= BASE_AST(expr->postfix_expr, validate_tree);
//         } break;
//         case AST_PREFIX_OP: {
//             ret &= BASE_AST(expr->prefix_op.rhs, validate_tree);
//         } break;
//         default: {
//             ret = false;
//             VALIDATION_ERR("Invalid tag (%d)", expr->tag);
//         }
//     }
//     return ret;
// }

// static void unary_expr_free(void *self) {
//     UnaryExpression expr = self;
//     switch (expr->tag) {
//         case AST_POSTFIX_EXPRESSION: {
//             BASE_AST(expr->postfix_expr, free_tree);
//         } break;
//         case AST_PREFIX_OP: {
//             BASE_AST(expr->prefix_op.rhs, free_tree);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     free(expr);
// }

// UnaryExpression ast_new_unary_expr(lineRange pos, PostfixExpression postfix_expr, TypeTagAST tag) {
//     UnaryExpression expr = malloc(sizeof(struct _unary_expression_t));
//     expr->tag = tag;
//     if (tag != AST_POSTFIX_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->postfix_expr = postfix_expr;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = unary_expr_print,
//         .free_tree = unary_expr_free,
//         .validate_tree = unary_expr_validate
//     };
//     return expr;
// }

// UnaryExpression ast_new_unary_expr_op(lineRange pos, UnaryExpression rhs, PrefixOp op, TypeTagAST tag) {
//     UnaryExpression expr = malloc(sizeof(struct _unary_expression_t));
//     expr->tag = tag;
//     if (tag != AST_PREFIX_OP) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->prefix_op.rhs = rhs;
//     expr->prefix_op.op = op;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = unary_expr_print,
//         .free_tree = unary_expr_free,
//         .validate_tree = unary_expr_validate
//     };
//     return expr;
// }

// #pragma endregion

// #pragma region PowerExpression

// static void power_expr_print(void *self, FILE *io_out, int indent) {
//     PowerExpression expr = self;
//     write_indent(io_out, indent);
//     fprintf(io_out, "PowerExpr(\n");
//     switch (expr->tag) {
//         case AST_UNARY_EXPRESSION: {
//             BASE_AST(expr->unary_expr, print_tree, io_out, indent + 1);
//         } break;
//         case AST_POWER_EXPRESSION: {
//             BASE_AST(expr->power_expr.lhs, print_tree, io_out, indent + 1);
//             BASE_AST(expr->power_expr.rhs, print_tree, io_out, indent + 1);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     write_indent(io_out, indent);
//     fprintf(io_out, ")\n");
// }

// static bool power_expr_validate(void *self) {
//     PowerExpression expr = self;
//     bool ret = true;
//     switch (expr->tag) {
//         case AST_UNARY_EXPRESSION: {
//             ret &= BASE_AST(expr->unary_expr, validate_tree);
//         } break;
//         case AST_POWER_EXPRESSION: {
//             ret &= BASE_AST(expr->power_expr.lhs, validate_tree);
//             ret &= BASE_AST(expr->power_expr.rhs, validate_tree);
//         } break;
//         default: {
//             ret = false;
//             VALIDATION_ERR("Invalid tag (%d)", expr->tag);
//         }
//     }
//     return ret;
// }

// static void power_expr_free(void *self) {
//     PowerExpression expr = self;
//     switch (expr->tag) {
//         case AST_UNARY_EXPRESSION: {
//             BASE_AST(expr->unary_expr, free_tree);
//         } break;
//         case AST_POWER_EXPRESSION: {
//             BASE_AST(expr->power_expr.lhs, free_tree);
//             BASE_AST(expr->power_expr.rhs, free_tree);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     free(expr);
// }

// PowerExpression ast_new_pow_expr_short(lineRange pos, UnaryExpression unary_expr, TypeTagAST tag) {
//     PowerExpression expr = malloc(sizeof(struct _power_expression_t));
//     expr->tag = tag;
//     if (tag != AST_UNARY_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->unary_expr = unary_expr;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = power_expr_print,
//         .free_tree = power_expr_free,
//         .validate_tree = power_expr_validate
//     };
//     return expr;
// }

// PowerExpression ast_new_pow_expr(lineRange pos, PowerExpression lhs, UnaryExpression rhs, TypeTagAST tag) {
//     PowerExpression expr = malloc(sizeof(struct _power_expression_t));
//     expr->tag = tag;
//     if (tag != AST_POWER_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->power_expr.lhs = lhs;
//     expr->power_expr.rhs = rhs;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = power_expr_print,
//         .free_tree = power_expr_free,
//         .validate_tree = power_expr_validate
//     };
//     return expr;
// }

// #pragma endregion

// #pragma region MultiplicativeExpression

// static void mul_expr_print(void *self, FILE *io_out, int indent) {
//     MultiplicativeExpression expr = self;
//     write_indent(io_out, indent);
//     fprintf(io_out, "MultiplicativeExpr(\n");
//     switch (expr->tag) {
//         case AST_POWER_EXPRESSION: {
//             BASE_AST(expr->power_expr, print_tree, io_out, indent + 1);
//         } break;
//         case AST_MULTIPLICATIVE_EXPRESSION: {
//             BASE_AST(expr->multiplicative_expr.lhs, print_tree, io_out, indent + 1);
//             BASE_AST(expr->multiplicative_expr.rhs, print_tree, io_out, indent + 1);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     write_indent(io_out, indent);
//     fprintf(io_out, ")\n");
// }

// static bool mul_expr_validate(void *self) {
//     MultiplicativeExpression expr = self;
//     bool ret = true;
//     switch (expr->tag) {
//         case AST_POWER_EXPRESSION: {
//             ret &= BASE_AST(expr->power_expr, validate_tree);
//         } break;
//         case AST_MULTIPLICATIVE_EXPRESSION: {
//             ret &= BASE_AST(expr->multiplicative_expr.lhs, validate_tree);
//             ret &= BASE_AST(expr->multiplicative_expr.rhs, validate_tree);
//         } break;
//         default: {
//             ret = false;
//             VALIDATION_ERR("Invalid tag (%d)", expr->tag);
//         }
//     }
//     return ret;
// }

// static void mul_expr_free(void *self) {
//     MultiplicativeExpression expr = self;
//     switch (expr->tag) {
//         case AST_POWER_EXPRESSION: {
//             BASE_AST(expr->power_expr, free_tree);
//         } break;
//         case AST_MULTIPLICATIVE_EXPRESSION: {
//             BASE_AST(expr->multiplicative_expr.lhs, free_tree);
//             BASE_AST(expr->multiplicative_expr.rhs, free_tree);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     free(expr);
// }

// MultiplicativeExpression ast_new_mul_expr_short(lineRange pos, PowerExpression power_expr, TypeTagAST tag) {
//     MultiplicativeExpression expr = malloc(sizeof(struct _multiplicative_expression_t));
//     expr->tag = tag;
//     if (tag != AST_POWER_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->power_expr = power_expr;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = mul_expr_print,
//         .free_tree = mul_expr_free,
//         .validate_tree = mul_expr_validate
//     };
//     return expr;
// }

// MultiplicativeExpression ast_new_mul_expr(lineRange pos, MultiplicativeExpression lhs, MultiplicativeOp op, PowerExpression rhs, TypeTagAST tag) {
//     MultiplicativeExpression expr = malloc(sizeof(struct _multiplicative_expression_t));
//     expr->tag = tag;
//     if (tag != AST_MULTIPLICATIVE_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->multiplicative_expr.lhs = lhs;
//     expr->multiplicative_expr.op = op;
//     expr->multiplicative_expr.rhs = rhs;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = mul_expr_print,
//         .free_tree = mul_expr_free,
//         .validate_tree = mul_expr_validate
//     };
//     return expr;
// }

// #pragma endregion

// #pragma region AdditiveExpression

// static void add_expr_print(void *self, FILE *io_out, int indent) {
//     AdditiveExpression expr = self;
//     write_indent(io_out, indent);
//     fprintf(io_out, "AdditiveExpr(\n");
//     switch (expr->tag) {
//         case AST_MULTIPLICATIVE_EXPRESSION: {
//             BASE_AST(expr->multiplicative_expr, print_tree, io_out, indent + 1);
//         } break;
//         case AST_ADDITIVE_EXPRESSION: {
//             BASE_AST(expr->additive_expr.lhs, print_tree, io_out, indent + 1);
//             BASE_AST(expr->additive_expr.rhs, print_tree, io_out, indent + 1);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     write_indent(io_out, indent);
//     fprintf(io_out, ")\n");
// }

// static bool add_expr_validate(void *self) {
//     AdditiveExpression expr = self;
//     bool ret = true;
//     switch (expr->tag) {
//         case AST_MULTIPLICATIVE_EXPRESSION: {
//             ret &= BASE_AST(expr->multiplicative_expr, validate_tree);
//         } break;
//         case AST_ADDITIVE_EXPRESSION: {
//             ret &= BASE_AST(expr->additive_expr.lhs, validate_tree);
//             ret &= BASE_AST(expr->additive_expr.rhs, validate_tree);
//         } break;
//         default: {
//             ret = false;
//             VALIDATION_ERR("Invalid tag (%d)", expr->tag);
//         }
//     }
//     return ret;
// }

// static void add_expr_free(void *self) {
//     AdditiveExpression expr = self;
//     switch (expr->tag) {
//         case AST_MULTIPLICATIVE_EXPRESSION: {
//             BASE_AST(expr->multiplicative_expr, free_tree);
//         } break;
//         case AST_ADDITIVE_EXPRESSION: {
//             BASE_AST(expr->additive_expr.lhs, free_tree);
//             BASE_AST(expr->additive_expr.rhs, free_tree);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     free(expr);
// }

// AdditiveExpression ast_new_add_expr_short(lineRange pos, MultiplicativeExpression mul_expr, TypeTagAST tag) {
//     AdditiveExpression expr = malloc(sizeof(struct _additive_expression_t));
//     expr->tag = tag;
//     if (tag != AST_MULTIPLICATIVE_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->multiplicative_expr = mul_expr;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = add_expr_print,
//         .free_tree = add_expr_free,
//         .validate_tree = add_expr_validate
//     };
//     return expr;
// }

// AdditiveExpression ast_new_add_expr(lineRange pos, AdditiveExpression lhs, AdditiveOp op, MultiplicativeExpression rhs, TypeTagAST tag) {
//     AdditiveExpression expr = malloc(sizeof(struct _additive_expression_t));
//     expr->tag = tag;
//     if (tag != AST_ADDITIVE_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->additive_expr.lhs = lhs;
//     expr->additive_expr.rhs = rhs;
//     expr->additive_expr.op = op;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = add_expr_print,
//         .free_tree = add_expr_free,
//         .validate_tree = add_expr_validate
//     };
//     return expr;
// }

// #pragma endregion

// #pragma region RelationalExpression

// static void relational_expr_print(void *self, FILE *io_out, int indent) {
//     RelationalExpression expr = self;
//     write_indent(io_out, indent);
//     fprintf(io_out, "RelationalExpr(\n");
//     switch (expr->tag) {
//         case AST_ADDITIVE_EXPRESSION: {
//             BASE_AST(expr->additive_expr, print_tree, io_out, indent + 1);
//         } break;
//         case AST_RELATIONAL_EXPRESSION: {
//             BASE_AST(expr->relational_expr.lhs, print_tree, io_out, indent + 1);
//             BASE_AST(expr->relational_expr.rhs, print_tree, io_out, indent + 1);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     write_indent(io_out, indent);
//     fprintf(io_out, ")\n");
// }

// static bool relational_expr_validate(void *self) {
//     RelationalExpression expr = self;
//     bool ret = true;
//     switch (expr->tag) {
//         case AST_ADDITIVE_EXPRESSION: {
//             ret &= BASE_AST(expr->additive_expr, validate_tree);
//         } break;
//         case AST_RELATIONAL_EXPRESSION: {
//             ret &= BASE_AST(expr->relational_expr.lhs, validate_tree);
//             ret &= BASE_AST(expr->relational_expr.rhs, validate_tree);
//         } break;
//         default: {
//             ret = false;
//             VALIDATION_ERR("Invalid tag (%d)", expr->tag);
//         }
//     }
//     return ret;
// }

// static void relational_expr_free(void *self) {
//     RelationalExpression expr = self;
//     switch (expr->tag) {
//         case AST_ADDITIVE_EXPRESSION: {
//             BASE_AST(expr->additive_expr, free_tree);
//         } break;
//         case AST_RELATIONAL_EXPRESSION: {
//             BASE_AST(expr->relational_expr.lhs, free_tree);
//             BASE_AST(expr->relational_expr.rhs, free_tree);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     free(expr);
// }

// RelationalExpression ast_new_relational_expr_short(lineRange pos, AdditiveExpression add_expr, TypeTagAST tag) {
//     RelationalExpression expr = malloc(sizeof(struct _relational_expression_t));
//     expr->tag = tag;
//     if (tag != AST_ADDITIVE_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->additive_expr = add_expr;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = relational_expr_print,
//         .free_tree = relational_expr_free,
//         .validate_tree = relational_expr_validate
//     };
//     return expr;
// }

// RelationalExpression ast_new_relational_expr(lineRange pos, RelationalExpression lhs, RelationalOp op, AdditiveExpression rhs, TypeTagAST tag) {
//     RelationalExpression expr = malloc(sizeof(struct _relational_expression_t));
//     expr->tag = tag;
//     if (tag != AST_RELATIONAL_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->relational_expr.lhs = lhs;
//     expr->relational_expr.op = op;
//     expr->relational_expr.rhs = rhs;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = relational_expr_print,
//         .free_tree = relational_expr_free,
//         .validate_tree = relational_expr_validate
//     };
//     return expr;
// }

// #pragma endregion

// #pragma region EqualityExpression

// static void equality_expr_print(void *self, FILE *io_out, int indent) {
//     EqualityExpression expr = self;
//     write_indent(io_out, indent);
//     fprintf(io_out, "EqualityExpr(\n");
//     switch (expr->tag) {
//         case AST_RELATIONAL_EXPRESSION: {
//             BASE_AST(expr->relational_expr, print_tree, io_out, indent + 1);
//         } break;
//         case AST_EQUALITY_EXPRESSION: {
//             BASE_AST(expr->equality_expr.lhs, print_tree, io_out, indent + 1);
//             BASE_AST(expr->equality_expr.rhs, print_tree, io_out, indent + 1);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     write_indent(io_out, indent);
//     fprintf(io_out, ")\n");
// }

// static bool equality_expr_validate(void *self) {
//     EqualityExpression expr = self;
//     bool ret = true;
//     switch (expr->tag) {
//         case AST_RELATIONAL_EXPRESSION: {
//             ret &= BASE_AST(expr->relational_expr, validate_tree);
//         } break;
//         case AST_EQUALITY_EXPRESSION: {
//             ret &= BASE_AST(expr->equality_expr.lhs, validate_tree);
//             ret &= BASE_AST(expr->equality_expr.rhs, validate_tree);
//         } break;
//         default: {
//             ret = false;
//             VALIDATION_ERR("Invalid tag (%d)", expr->tag);
//         }
//     }
//     return ret;
// }

// static void equality_expr_free(void *self) {
//     EqualityExpression expr = self;
//     switch (expr->tag) {
//         case AST_RELATIONAL_EXPRESSION: {
//             BASE_AST(expr->relational_expr, free_tree);
//         } break;
//         case AST_EQUALITY_EXPRESSION: {
//             BASE_AST(expr->equality_expr.lhs, free_tree);
//             BASE_AST(expr->equality_expr.rhs, free_tree);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     free(expr);
// }

// EqualityExpression ast_new_eq_expr_short(lineRange pos, RelationalExpression rel_expr, TypeTagAST tag) {
//     EqualityExpression expr = malloc(sizeof(struct _equality_expression_t));
//     expr->tag = tag;
//     if (tag != AST_RELATIONAL_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->relational_expr = rel_expr;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = equality_expr_print,
//         .free_tree = equality_expr_free,
//         .validate_tree = equality_expr_validate
//     };
//     return expr;
// }

// EqualityExpression ast_new_eq_expr(lineRange pos, EqualityExpression lhs, EqualityOp op, RelationalExpression rhs, TypeTagAST tag) {
//     EqualityExpression expr = malloc(sizeof(struct _equality_expression_t));
//     expr->tag = tag;
//     if (tag != AST_EQUALITY_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->equality_expr.lhs = lhs;
//     expr->equality_expr.op = op;
//     expr->equality_expr.rhs = rhs;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = equality_expr_print,
//         .free_tree = equality_expr_free,
//         .validate_tree = equality_expr_validate
//     };
//     return expr;
// }

// #pragma endregion

// #pragma region LogicalAndExpression

// static void logic_and_expr_print(void *self, FILE *io_out, int indent) {
//     LogicalAndExpression expr = self;
//     write_indent(io_out, indent);
//     fprintf(io_out, "LogicalAndExpr(\n");
//     switch (expr->tag) {
//         case AST_EQUALITY_EXPRESSION: {
//             BASE_AST(expr->equality_expr, print_tree, io_out, indent + 1);
//         } break;
//         case AST_LOGICAL_AND_EXPRESSION: {
//             BASE_AST(expr->logical_and_expr.lhs, print_tree, io_out, indent + 1);
//             BASE_AST(expr->logical_and_expr.rhs, print_tree, io_out, indent + 1);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     write_indent(io_out, indent);
//     fprintf(io_out, ")\n");
// }

// static bool logic_and_expr_validate(void *self) {
//     LogicalAndExpression expr = self;
//     bool ret = true;
//     switch (expr->tag) {
//         case AST_EQUALITY_EXPRESSION: {
//             ret &= BASE_AST(expr->equality_expr, validate_tree);
//         } break;
//         case AST_LOGICAL_AND_EXPRESSION: {
//             ret &= BASE_AST(expr->logical_and_expr.lhs, validate_tree);
//             ret &= BASE_AST(expr->logical_and_expr.rhs, validate_tree);
//         } break;
//         default: {
//             ret = false;
//             VALIDATION_ERR("Invalid tag (%d)", expr->tag);
//         }
//     }
//     return ret;
// }

// static void logic_and_expr_free(void *self) {
//     LogicalAndExpression expr = self;
//     switch (expr->tag) {
//         case AST_EQUALITY_EXPRESSION: {
//             BASE_AST(expr->equality_expr, free_tree);
//         } break;
//         case AST_LOGICAL_AND_EXPRESSION: {
//             BASE_AST(expr->logical_and_expr.lhs, free_tree);
//             BASE_AST(expr->logical_and_expr.rhs, free_tree);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     free(expr);
// }

// LogicalAndExpression ast_new_logic_and_expr_short(lineRange pos, EqualityExpression eq_expr, TypeTagAST tag) {
//     LogicalAndExpression expr = malloc(sizeof(struct _logical_and_expression_t));
//     expr->tag = tag;
//     if (tag != AST_EQUALITY_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->equality_expr = eq_expr;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = logic_and_expr_print,
//         .free_tree = logic_and_expr_free,
//         .validate_tree = logic_and_expr_validate
//     };
//     return expr;
// }

// LogicalAndExpression ast_new_logic_and_expr(lineRange pos, LogicalAndExpression lhs, EqualityExpression rhs, TypeTagAST tag) {
//     LogicalAndExpression expr = malloc(sizeof(struct _logical_and_expression_t));
//     expr->tag = tag;
//     if (tag != AST_LOGICAL_AND_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->logical_and_expr.lhs = lhs;
//     expr->logical_and_expr.rhs = rhs;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = logic_and_expr_print,
//         .free_tree = logic_and_expr_free,
//         .validate_tree = logic_and_expr_validate
//     };
//     return expr;
// }

// #pragma endregion

// #pragma region LogicalOrExpression

// static void logic_or_expr_print(void *self, FILE *io_out, int indent) {
//     LogicalOrExpression expr = self;
//     write_indent(io_out, indent);
//     fprintf(io_out, "LogicalOrExpr(\n");
//     switch (expr->tag) {
//         case AST_LOGICAL_AND_EXPRESSION: {
//             BASE_AST(expr->logical_and_expr, print_tree, io_out, indent + 1);
//         } break;
//         case AST_LOGICAL_OR_EXPRESSION: {
//             BASE_AST(expr->logical_or_expr.lhs, print_tree, io_out, indent + 1);
//             BASE_AST(expr->logical_or_expr.rhs, print_tree, io_out, indent + 1);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     write_indent(io_out, indent);
//     fprintf(io_out, ")\n");
// }

// static bool logic_or_expr_validate(void *self) {
//     LogicalOrExpression expr = self;
//     bool ret = true;
//     switch (expr->tag) {
//         case AST_LOGICAL_AND_EXPRESSION: {
//             ret &= BASE_AST(expr->logical_and_expr, validate_tree);
//         } break;
//         case AST_LOGICAL_OR_EXPRESSION: {
//             ret &= BASE_AST(expr->logical_or_expr.lhs, validate_tree);
//             ret &= BASE_AST(expr->logical_or_expr.rhs, validate_tree);
//         } break;
//         default: {
//             ret = false;
//             VALIDATION_ERR("Invalid tag (%d)", expr->tag);
//         }
//     }
//     return ret;
// }

// static void logic_or_expr_free(void *self) {
//     LogicalOrExpression expr = self;
//     switch (expr->tag) {
//         case AST_LOGICAL_AND_EXPRESSION: {
//             BASE_AST(expr->logical_and_expr, free_tree);
//         } break;
//         case AST_LOGICAL_OR_EXPRESSION: {
//             BASE_AST(expr->logical_or_expr.lhs, free_tree);
//             BASE_AST(expr->logical_or_expr.rhs, free_tree);
//         } break;
//         default: {
//             AST_TAG_ERR(expr);
//             exit(1);
//         }
//     }
//     free(expr);
// }

// LogicalOrExpression ast_new_logic_or_expr_short(lineRange pos, LogicalAndExpression logic_and_expr, TypeTagAST tag) {
//     LogicalOrExpression expr = malloc(sizeof(struct _logical_or_expression_t));
//     expr->tag = tag;
//     if (tag != AST_LOGICAL_AND_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->logical_and_expr = logic_and_expr;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = logic_or_expr_print,
//         .free_tree = logic_or_expr_free,
//         .validate_tree = logic_or_expr_validate
//     };
//     return expr;
// }

// LogicalOrExpression ast_new_logic_or_expr(lineRange pos, LogicalOrExpression lhs, LogicalAndExpression rhs, TypeTagAST tag) {
//     LogicalOrExpression expr = malloc(sizeof(struct _logical_or_expression_t));
//     expr->tag = tag;
//     if (tag != AST_LOGICAL_OR_EXPRESSION) {
//         AST_TAG_ERR(expr);
//         exit(1);
//     }
//     expr->logical_or_expr.lhs = lhs;
//     expr->logical_or_expr.rhs = rhs;
//     expr->parent = (baseAST) {
//         .position = pos,
//         .print_tree = logic_or_expr_print,
//         .free_tree = logic_or_expr_free,
//         .validate_tree = logic_or_expr_validate
//     };
//     return expr;
// }

// ConditionalExpression ast_new_conditional_expr(lineRange pos, LogicalOrExpression expr);

// ConditionalExpression ast_new_conditional_expr_tern(lineRange pos, LogicalOrExpression expr, Expression if_expr, Expression else_expr);

// Block ast_new_block(lineRange pos, Vector(TopLevelExpression) topLevelExpressions);

// Expression ast_new_expr_gen(lineRange pos, void *arg, TypeTagAST tag);

// Expression ast_new_expr_func_block(lineRange pos, FuncDefinition func_def, Block block, TypeTagAST tag);

// Expression ast_new_expr_tuple_block(lineRange pos, TupleDefinition tuple_def, Block block, TypeTagAST tag);

// CompoundConditionals ast_new_compound_conditional(lineRange pos, void *arg, TypeTagAST tag);

// WhileLoop ast_new_while_loop(lineRange pos, ConditionalExpression cond_expr, Block block, OPT(Block) else_block);

// ForLoop ast_new_for_loop(lineRange pos, ForLoopContents contents, Block block, OPT(Block) else_block);

// ForLoopContents ast_new_for_loop_contents_in(lineRange pos, Vector(char*) ids, ConditionalExpression cond_expr, TypeTagAST tag);

// ForLoopContents ast_new_for_loop_contents_in(lineRange pos, Vector(char*) ids, ConditionalExpression cond_expr, TypeTagAST tag);

// IfBlock ast_new_if_block(lineRange pos, ConditionalExpression cond, Block block, OPT(Block) else_block);

// ElseBlock ast_new_else_block(lineRange pos, void *arg, TypeTagAST tag);

// TupleMember ast_new_tuple_member(lineRange pos, char *id, OPT(TypeExpression) type);

// TupleDefinition ast_new_tuple_def(lineRange pos, Vector(TupleMember) members);

// TypeMember ast_new_type_member_gen(lineRange pos, void *arg, TypeTagAST tag);

// TypeMember ast_new_type_member_array_map(lineRange pos, TypeExpression lhs, OPT(TypeExpression) rhs, TypeTagAST tag);

// TypeExpression ast_new_type_expr(lineRange pos, TypeMember lhs, OPT(TypeExpression) or_type, OPT(TypeExpression) impl_type);

// FuncDefinition ast_new_func_def(lineRange pos, OPT(TupleDefinition) def, OPT(TypeExpression) ret_type);

// Constant ast_new_constant(lineRange pos, void *arg, TypeTagAST tag);

// MapConstant ast_new_map_constant(lineRange pos, Vector(Expression) keys, Vector(Expression) values);

// ArrayConstant ast_new_array_constant(lineRange pos, Vector(Expression) contents);
