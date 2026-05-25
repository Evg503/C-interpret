#include <stdio.h>
#include <string.h>
#include "include/lexer.h"
#include "unity.h"

#include "src/lexer.c"

void test_basic_tokens(void) {
    const char* source = "int x = 10;";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    Token token = get_next_token(&lexer);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_INT, token.type);
    free_token(&token);
    
    token = get_next_token(&lexer);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_IDENTIFIER, token.type);
    TEST_ASSERT_EQUAL_STRING("x", token.value);
    free_token(&token);
    
    token = get_next_token(&lexer);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_ASSIGN, token.type);
    free_token(&token);
    
    token = get_next_token(&lexer);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_NUMBER, token.type);
    TEST_ASSERT_EQUAL_STRING("10", token.value);
    free_token(&token);
    
    token = get_next_token(&lexer);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_SEMICOLON, token.type);
    free_token(&token);
    
    token = get_next_token(&lexer);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_EOF, token.type);
    free_token(&token);
}

void test_operators(void) {
    const char* source = "+ - * / = == != < > <= >= && || !";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    TEST_ASSERT_EQUAL_HEX8(TOKEN_PLUS, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_MINUS, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_STAR, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_SLASH, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_ASSIGN, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_EQ, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_NEQ, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_LT, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_GT, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_LE, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_GE, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_AND, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_OR, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_NOT, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_EOF, get_next_token(&lexer).type);
}

void test_compounded_operators(void) {
    const char* source = "++ -- << >> && ||";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    TEST_ASSERT_EQUAL_HEX8(TOKEN_PLUS_PLUS, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_MINUS_MINUS, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_SHIFT_LEFT, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_SHIFT_RIGHT, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_AND, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_OR, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_EOF, get_next_token(&lexer).type);
}

void test_keywords(void) {
    const char* source = "if else while for return int char void print break continue";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    TEST_ASSERT_EQUAL_HEX8(TOKEN_IF, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_ELSE, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_WHILE, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_FOR, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_RETURN, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_INT, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_CHAR, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_VOID, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_PRINT, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_BREAK, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_CONTINUE, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_EOF, get_next_token(&lexer).type);
}

void test_strings_and_comments(void) {
    const char* source = "print(\"hello\"); // comment";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    TEST_ASSERT_EQUAL_HEX8(TOKEN_PRINT, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_LPAREN, get_next_token(&lexer).type);
    
    Token token = get_next_token(&lexer);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_STRING, token.type);
    TEST_ASSERT_EQUAL_STRING("hello", token.value);
    free_token(&token);
    
    TEST_ASSERT_EQUAL_HEX8(TOKEN_RPAREN, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_SEMICOLON, get_next_token(&lexer).type);
    TEST_ASSERT_EQUAL_HEX8(TOKEN_EOF, get_next_token(&lexer).type);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_basic_tokens);
    RUN_TEST(test_operators);
    RUN_TEST(test_compounded_operators);
    RUN_TEST(test_keywords);
    RUN_TEST(test_strings_and_comments);
    
    return UNITY_END();
}

void setUp(void) {
}

void tearDown(void) {
}

