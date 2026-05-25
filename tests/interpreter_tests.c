#define UNITY_SUPPORT_64
#include "unity.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

void setUp(void) {
    init_interpreter();
}

void tearDown(void) {
}

void test_set_and_get_variable(void) {
    set_variable("x", 42);
    int value = get_variable("x");
    TEST_ASSERT_EQUAL(42, value);
}

void test_arithmetic_addition(void) {
    Lexer lexer;
    init_lexer(&lexer, "3 + 5;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(8, result);
    free_ast(ast);
}

void test_arithmetic_subtraction(void) {
    Lexer lexer;
    init_lexer(&lexer, "10 - 4;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(6, result);
    free_ast(ast);
}

void test_arithmetic_multiplication(void) {
    Lexer lexer;
    init_lexer(&lexer, "6 * 7;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(42, result);
    free_ast(ast);
}

void test_arithmetic_division(void) {
    Lexer lexer;
    init_lexer(&lexer, "20 / 4;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(5, result);
    free_ast(ast);
}

void test_arithmetic_operator_precedence(void) {
    Lexer lexer;
    init_lexer(&lexer, "2 + 3 * 4;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(14, result);
    free_ast(ast);
}

void test_arithmetic_with_parentheses(void) {
    Lexer lexer;
    init_lexer(&lexer, "(2 + 3) * 4;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(20, result);
    free_ast(ast);
}

void test_assignment_operation(void) {
    Lexer lexer;
    init_lexer(&lexer, "x = 15;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(15, result);
    TEST_ASSERT_EQUAL(15, get_variable("x"));
    free_ast(ast);
}

void test_variable_in_expression(void) {
    set_variable("a", 10);
    set_variable("b", 5);
    Lexer lexer;
    init_lexer(&lexer, "a + b;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(15, result);
    free_ast(ast);
}

void test_complex_expression(void) {
    set_variable("x", 8);
    Lexer lexer;
    init_lexer(&lexer, "x * 2 + 6 / 3;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(18, result);
    free_ast(ast);
}

void test_comparison_less_than(void) {
    Lexer lexer;
    init_lexer(&lexer, "3 < 5;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(1, result);
    free_ast(ast);
}

void test_comparison_greater_than(void) {
    Lexer lexer;
    init_lexer(&lexer, "10 > 3;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(1, result);
    free_ast(ast);
}

void test_comparison_equal(void) {
    Lexer lexer;
    init_lexer(&lexer, "7 == 7;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(1, result);
    free_ast(ast);
}

void test_comparison_not_equal(void) {
    Lexer lexer;
    init_lexer(&lexer, "4 != 6;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(1, result);
    free_ast(ast);
}

void test_logical_and(void) {
    Lexer lexer;
    init_lexer(&lexer, "1 && 1;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(1, result);
    free_ast(ast);
}

void test_logical_or(void) {
    Lexer lexer;
    init_lexer(&lexer, "0 || 1;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(1, result);
    free_ast(ast);
}

void test_logical_not(void) {
    Lexer lexer;
    init_lexer(&lexer, "!0;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(1, result);
    free_ast(ast);
}

void test_unary_minus(void) {
    Lexer lexer;
    init_lexer(&lexer, "-5;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(-5, result);
    free_ast(ast);
}

void test_unary_plus(void) {
    Lexer lexer;
    init_lexer(&lexer, "+10;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(10, result);
    free_ast(ast);
}

void test_ternary_operator_true(void) {
    Lexer lexer;
    init_lexer(&lexer, "1 ? 100 : 200;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(100, result);
    free_ast(ast);
}

void test_ternary_operator_false(void) {
    Lexer lexer;
    init_lexer(&lexer, "0 ? 100 : 200;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    int result = interpret_ast(ast);
    TEST_ASSERT_EQUAL(200, result);
    free_ast(ast);
}

void test_multiple_statements(void) {
    Lexer lexer;
    init_lexer(&lexer, "x = 5; y = 10;");
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    interpret_ast(ast);
    TEST_ASSERT_EQUAL(10, get_variable("y"));
    free_ast(ast);
}

void test_complex_program_execution(void) {
    const char* code = "x = 5;\ny = 10;\nz = x + y * 2;";
    
    Lexer lexer;
    init_lexer(&lexer, code);
    Parser parser;
    init_parser(&parser, &lexer);
    ASTNode* ast = parse_program(&parser);
    interpret_ast(ast);
    
    TEST_ASSERT_EQUAL(5, get_variable("x"));
    TEST_ASSERT_EQUAL(10, get_variable("y"));
    TEST_ASSERT_EQUAL(25, get_variable("z"));
    
    free_ast(ast);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_set_and_get_variable);
    RUN_TEST(test_arithmetic_addition);
    RUN_TEST(test_arithmetic_subtraction);
    RUN_TEST(test_arithmetic_multiplication);
    RUN_TEST(test_arithmetic_division);
    RUN_TEST(test_arithmetic_operator_precedence);
    RUN_TEST(test_arithmetic_with_parentheses);
    RUN_TEST(test_assignment_operation);
    RUN_TEST(test_variable_in_expression);
    RUN_TEST(test_complex_expression);
    RUN_TEST(test_comparison_less_than);
    RUN_TEST(test_comparison_greater_than);
    RUN_TEST(test_comparison_equal);
    RUN_TEST(test_comparison_not_equal);
    RUN_TEST(test_logical_and);
    RUN_TEST(test_logical_or);
    RUN_TEST(test_logical_not);
    RUN_TEST(test_unary_minus);
    RUN_TEST(test_unary_plus);
    RUN_TEST(test_ternary_operator_true);
    RUN_TEST(test_ternary_operator_false);
    RUN_TEST(test_multiple_statements);
    RUN_TEST(test_complex_program_execution);
    
    return UNITY_END();
}
