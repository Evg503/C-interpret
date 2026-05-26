#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "include/parser.h"
#include "include/lexer.h"
#include "src/lexer.c"
#include "src/parser.c"

void test_parse_number(void) {
    const char* source = "123;";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    TEST_ASSERT_EQUAL(NODE_STATEMENT_LIST, program->type);
    TEST_ASSERT_EQUAL(1, program->statement_list.count);
    
    ASTNode* stmt = program->statement_list.statements[0];
    TEST_ASSERT_EQUAL(NODE_EXPRESSION_STATEMENT, stmt->type);
    ASTNode* expr = stmt->expr_stmt.expression;
    TEST_ASSERT_EQUAL(NODE_NUMBER, expr->type);
    TEST_ASSERT_EQUAL(123, expr->number.value);
    
    free_ast(program);
}

void test_parse_variable_declaration(void) {
    const char* source = "int x = 10;";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    TEST_ASSERT_EQUAL(NODE_STATEMENT_LIST, program->type);
    TEST_ASSERT_EQUAL(1, program->statement_list.count);
    
    ASTNode* stmt = program->statement_list.statements[0];
    TEST_ASSERT_EQUAL(NODE_VARIABLE_DECL, stmt->type);
    TEST_ASSERT_EQUAL_STRING("x", stmt->var_decl.var_name);
    TEST_ASSERT_NOT_NULL(stmt->var_decl.initializer);
    TEST_ASSERT_EQUAL(NODE_NUMBER, stmt->var_decl.initializer->type);
    TEST_ASSERT_EQUAL(10, stmt->var_decl.initializer->number.value);
    
    free_ast(program);
}

void test_parse_uninitialized_variable(void) {
    const char* source = "int x;";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    ASTNode* stmt = program->statement_list.statements[0];
    
    TEST_ASSERT_EQUAL(NODE_VARIABLE_DECL, stmt->type);
    TEST_ASSERT_EQUAL_STRING("x", stmt->var_decl.var_name);
    TEST_ASSERT_NULL(stmt->var_decl.initializer);
    
    free_ast(program);
}

void test_parse_assignment(void) {
    const char* source = "x = 10;";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    ASTNode* stmt = program->statement_list.statements[0];
    
    TEST_ASSERT_EQUAL(NODE_EXPRESSION_STATEMENT, stmt->type);
    TEST_ASSERT_EQUAL(NODE_ASSIGNMENT, stmt->expr_stmt.expression->type);
    ASTNode* assign = stmt->expr_stmt.expression;
    TEST_ASSERT_EQUAL_STRING("x", assign->assignment.var_name);
    TEST_ASSERT_EQUAL(NODE_NUMBER, assign->assignment.expression->type);
    
    free_ast(program);
}

void test_parse_binary_expression(void) {
    const char* source = "x + y * 2;";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    ASTNode* stmt = program->statement_list.statements[0];
    
    TEST_ASSERT_EQUAL(NODE_EXPRESSION_STATEMENT, stmt->type);
    TEST_ASSERT_EQUAL(NODE_BINARY_OP, stmt->expr_stmt.expression->type);
    ASTNode* expr = stmt->expr_stmt.expression;
    TEST_ASSERT_EQUAL(TOKEN_PLUS, expr->binary.op);
    
    TEST_ASSERT_EQUAL(NODE_IDENTIFIER, expr->binary.left->type);
    TEST_ASSERT_EQUAL_STRING("x", expr->binary.left->identifier.name);
    
    TEST_ASSERT_EQUAL(NODE_BINARY_OP, expr->binary.right->type);
    TEST_ASSERT_EQUAL(TOKEN_STAR, expr->binary.right->binary.op);
    
    free_ast(program);
}

void test_parse_if_statement(void) {
    const char* source = "if (x > 0) { print(x); }";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    TEST_ASSERT_EQUAL(NODE_STATEMENT_LIST, program->type);
    TEST_ASSERT_EQUAL(1, program->statement_list.count);
    
    ASTNode* stmt = program->statement_list.statements[0];
    TEST_ASSERT_EQUAL(NODE_IF_STATEMENT, stmt->type);
    TEST_ASSERT_NOT_NULL(stmt->if_stmt.condition);
    TEST_ASSERT_NOT_NULL(stmt->if_stmt.then_branch);
    TEST_ASSERT_NULL(stmt->if_stmt.else_branch);
    
    free_ast(program);
}

void test_parse_if_else_statement(void) {
    const char* source = "if (x > 0) { y = 1; } else { y = 2; }";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    ASTNode* stmt = program->statement_list.statements[0];
    
    TEST_ASSERT_EQUAL(NODE_IF_STATEMENT, stmt->type);
    TEST_ASSERT_NOT_NULL(stmt->if_stmt.else_branch);
    
    free_ast(program);
}

void test_parse_while_statement(void) {
    const char* source = "while (x < 10) { x = x + 1; }";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    ASTNode* stmt = program->statement_list.statements[0];
    
    TEST_ASSERT_EQUAL(NODE_WHILE_STATEMENT, stmt->type);
    TEST_ASSERT_NOT_NULL(stmt->while_stmt.condition);
    TEST_ASSERT_NOT_NULL(stmt->while_stmt.body);
    
    free_ast(program);
}

// TODO: Add for statement support to parser
// void test_parse_for_statement(void) {
//     const char* source = "for (i = 0; i < 10; i = i + 1) { print(i); }";
//     Lexer lexer;
//     init_lexer(&lexer, source);
//     Parser parser;
//     init_parser(&parser, &lexer);
    
//     ASTNode* program = parse_program(&parser);
//     ASTNode* stmt = program->statement_list.statements[0];
    
//     TEST_ASSERT_EQUAL(NODE_FOR_STATEMENT, stmt->type);
//     TEST_ASSERT_NOT_NULL(stmt->for_stmt.init);
//     TEST_ASSERT_NOT_NULL(stmt->for_stmt.condition);
//     TEST_ASSERT_NOT_NULL(stmt->for_stmt.increment);
//     TEST_ASSERT_NOT_NULL(stmt->for_stmt.body);
    
//     free_ast(program);
// }

void test_parse_print_statement(void) {
    const char* source = "print(x + y);";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    ASTNode* stmt = program->statement_list.statements[0];
    
    TEST_ASSERT_EQUAL(NODE_PRINT_STATEMENT, stmt->type);
    TEST_ASSERT_NOT_NULL(stmt->print_stmt.expression);
    
    free_ast(program);
}

void test_parse_return_statement(void) {
    const char* source = "return 42;";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    ASTNode* stmt = program->statement_list.statements[0];
    
    TEST_ASSERT_EQUAL(NODE_RETURN_STATEMENT, stmt->type);
    TEST_ASSERT_NOT_NULL(stmt->return_stmt.expression);
    TEST_ASSERT_EQUAL(NODE_NUMBER, stmt->return_stmt.expression->type);
    
    free_ast(program);
}

void test_parse_block_statement(void) {
    const char* source = "{ int x = 1; int y = 2; }";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    TEST_ASSERT_EQUAL(NODE_STATEMENT_LIST, program->type);
    TEST_ASSERT_EQUAL(1, program->statement_list.count);
    
    ASTNode* stmt = program->statement_list.statements[0];
    TEST_ASSERT_EQUAL(NODE_STATEMENT_LIST, stmt->type);
    TEST_ASSERT_EQUAL(2, stmt->statement_list.count);
    
    free_ast(program);
}

void test_parse_ternary_operator(void) {
    const char* source = "x > 0 ? x : -x;";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    ASTNode* stmt = program->statement_list.statements[0];
    
    TEST_ASSERT_EQUAL(NODE_EXPRESSION_STATEMENT, stmt->type);
    TEST_ASSERT_EQUAL(NODE_TERNARY_OP, stmt->expr_stmt.expression->type);
    ASTNode* ternary = stmt->expr_stmt.expression;
    TEST_ASSERT_NOT_NULL(ternary->ternary.condition);
    TEST_ASSERT_NOT_NULL(ternary->ternary.true_expr);
    TEST_ASSERT_NOT_NULL(ternary->ternary.false_expr);
    
    free_ast(program);
}

void test_parse_function_call(void) {
    const char* source = "max(10, 20);";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    ASTNode* stmt = program->statement_list.statements[0];
    
    TEST_ASSERT_EQUAL(NODE_EXPRESSION_STATEMENT, stmt->type);
    TEST_ASSERT_EQUAL(NODE_CALL, stmt->expr_stmt.expression->type);
    ASTNode* call = stmt->expr_stmt.expression;
    TEST_ASSERT_EQUAL_STRING("max", call->call.func_name);
    TEST_ASSERT_EQUAL(2, call->call.arg_count);
    
    free_ast(program);
}

void test_parse_break_statement(void) {
    const char* source = "break;";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    TEST_ASSERT_EQUAL(NODE_STATEMENT_LIST, program->type);
    TEST_ASSERT_EQUAL(1, program->statement_list.count);
    
    ASTNode* stmt = program->statement_list.statements[0];
    TEST_ASSERT_EQUAL(NODE_BREAK_STATEMENT, stmt->type);
    
    free_ast(program);
}

void test_parse_continue_statement(void) {
    const char* source = "continue;";
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    ASTNode* stmt = program->statement_list.statements[0];
    
    TEST_ASSERT_EQUAL(NODE_CONTINUE_STATEMENT, stmt->type);
    
    free_ast(program);
}

void test_parse_complex_program(void) {
    const char* source = 
        "int sum = 0;\n"
        "for (i = 0; i < 10; i = i + 1) {\n"
        "    if (i % 2 == 0) {\n"
        "        sum = sum + i;\n"
        "    }\n"
        "}\n"
        "print(sum);";
    
    Lexer lexer;
    init_lexer(&lexer, source);
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* program = parse_program(&parser);
    
    TEST_ASSERT_EQUAL(NODE_STATEMENT_LIST, program->type);
    TEST_ASSERT_EQUAL(3, program->statement_list.count);
    
    free_ast(program);
}

void setUp(void) {
}

void tearDown(void) {
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_parse_number);
    RUN_TEST(test_parse_variable_declaration);
    RUN_TEST(test_parse_uninitialized_variable);
    RUN_TEST(test_parse_assignment);
    RUN_TEST(test_parse_binary_expression);
    RUN_TEST(test_parse_if_statement);
    RUN_TEST(test_parse_if_else_statement);
    RUN_TEST(test_parse_while_statement);
    // RUN_TEST(test_parse_for_statement);  // TODO: Add for statement support
    RUN_TEST(test_parse_print_statement);
    RUN_TEST(test_parse_return_statement);
    RUN_TEST(test_parse_block_statement);
    RUN_TEST(test_parse_ternary_operator);
    RUN_TEST(test_parse_function_call);
    RUN_TEST(test_parse_break_statement);
    RUN_TEST(test_parse_continue_statement);
    RUN_TEST(test_parse_complex_program);
    
    return UNITY_END();
}
