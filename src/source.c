///## Полный парсер выражений с приоритетами и скобками
///## Добавим следующие возможности:

///## Приоритет операций (как в C)

///## Скобки для изменения приоритета

///## Унарные операции: +, -, !

///## Бинарные операции: арифметические, сравнения, логические, битовые


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Типы токенов (расширенные)
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    
    // Операторы
    TOKEN_ASSIGN,       // =
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_STAR,         // *
    TOKEN_SLASH,        // /
    TOKEN_PERCENT,      // %
    TOKEN_PLUS_PLUS,    // ++
    TOKEN_MINUS_MINUS,  // --
    
    // Сравнение
    TOKEN_EQ,           // ==
    TOKEN_NEQ,          // !=
    TOKEN_LT,           // <
    TOKEN_GT,           // >
    TOKEN_LE,           // <=
    TOKEN_GE,           // >=
    
    // Логические
    TOKEN_AND,          // &&
    TOKEN_OR,           // ||
    TOKEN_NOT,          // !
    
    // Битовые
    TOKEN_BIT_AND,      // &
    TOKEN_BIT_OR,       // |
    TOKEN_BIT_XOR,      // ^
    TOKEN_BIT_NOT,      // ~
    TOKEN_SHIFT_LEFT,   // <<
    TOKEN_SHIFT_RIGHT,  // >>
    
    // Разделители
    TOKEN_SEMICOLON,    // ;
    TOKEN_COMMA,        // ,
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
    TOKEN_LBRACKET,     // [
    TOKEN_RBRACKET,     // ]
    
    // Ключевые слова
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,
    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_VOID,
    TOKEN_PRINT,
    TOKEN_BREAK,
    TOKEN_CONTINUE
} TokenType;

// Структура токена
typedef struct {
    TokenType type;
    char* value;
    int line;
    int column;
} Token;

// Структура лексера
typedef struct {
    const char* source;
    int pos;
    int line;
    int col;
    Token current_token;
} Lexer;

// Ключевые слова
typedef struct {
    const char* word;
    TokenType type;
} Keyword;

Keyword keywords[] = {
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"for", TOKEN_FOR},
    {"return", TOKEN_RETURN},
    {"int", TOKEN_INT},
    {"char", TOKEN_CHAR},
    {"void", TOKEN_VOID},
    {"print", TOKEN_PRINT},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {NULL, TOKEN_EOF}
};

// Типы узлов AST для выражений
typedef enum {
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_STRING,
    NODE_UNARY_OP,
    NODE_BINARY_OP,
    NODE_TERNARY_OP,    // ? :
    NODE_ASSIGNMENT,
    NODE_CALL,
    NODE_VARIABLE_DECL,
    NODE_STATEMENT_LIST,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_FOR_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_PRINT_STATEMENT,
    NODE_BLOCK
} NodeType;

// Структура AST узла
typedef struct ASTNode {
    NodeType type;
    int line;
    int column;
    union {
        struct {
            int value;
        } number;
        struct {
            char* name;
        } identifier;
        struct {
            char* value;
        } string;
        struct {
            int op;  // тип операции
            struct ASTNode* operand;
        } unary;
        struct {
            int op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary;
        struct {
            struct ASTNode* condition;
            struct ASTNode* true_expr;
            struct ASTNode* false_expr;
        } ternary;
        struct {
            char* var_name;
            struct ASTNode* expression;
        } assignment;
        struct {
            char* func_name;
            struct ASTNode** args;
            int arg_count;
        } call;
        struct {
            char* var_name;
            struct ASTNode* initializer;
            int type;  // INT, CHAR и т.д.
        } var_decl;
        struct {
            struct ASTNode** statements;
            int count;
            int capacity;
        } statement_list;
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch;
        } if_stmt;
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_stmt;
        struct {
            struct ASTNode* init;
            struct ASTNode* condition;
            struct ASTNode* increment;
            struct ASTNode* body;
        } for_stmt;
        struct {
            struct ASTNode* expression;
        } return_stmt;
        struct {
            struct ASTNode* expression;
        } print_stmt;
    };
} ASTNode;

// Структура парсера
typedef struct {
    Lexer* lexer;
    Token current;
} Parser;

// Приоритеты операций (чем выше число, тем выше приоритет)
typedef struct {
    int precedence;
    int associativity; // 0 = left, 1 = right
} Precedence;

Precedence get_precedence(TokenType op) {
    switch (op) {
        // Тернарный оператор (низший приоритет)
        case TOKEN_ASSIGN: return (Precedence){1, 1}; // right-associative
        
        // Логические операторы
        case TOKEN_OR: return (Precedence){2, 0};
        case TOKEN_AND: return (Precedence){3, 0};
        
        // Битовые OR
        case TOKEN_BIT_OR: return (Precedence){4, 0};
        case TOKEN_BIT_XOR: return (Precedence){5, 0};
        case TOKEN_BIT_AND: return (Precedence){6, 0};
        
        // Сравнение
        case TOKEN_EQ: case TOKEN_NEQ: return (Precedence){7, 0};
        case TOKEN_LT: case TOKEN_GT: case TOKEN_LE: case TOKEN_GE: 
            return (Precedence){8, 0};
        
        // Сдвиги
        case TOKEN_SHIFT_LEFT: case TOKEN_SHIFT_RIGHT: 
            return (Precedence){9, 0};
        
        // Сложение/вычитание
        case TOKEN_PLUS: case TOKEN_MINUS: 
            return (Precedence){10, 0};
        
        // Умножение/деление/остаток
        case TOKEN_STAR: case TOKEN_SLASH: case TOKEN_PERCENT: 
            return (Precedence){11, 0};
        
        // Унарные операторы (высший приоритет)
        default: return (Precedence){0, 0};
    }
}

// Прототипы функций парсера
ASTNode* parse_program(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_assignment(Parser* parser);
ASTNode* parse_ternary(Parser* parser);
ASTNode* parse_logical_or(Parser* parser);
ASTNode* parse_logical_and(Parser* parser);
ASTNode* parse_bitwise_or(Parser* parser);
ASTNode* parse_bitwise_xor(Parser* parser);
ASTNode* parse_bitwise_and(Parser* parser);
ASTNode* parse_equality(Parser* parser);
ASTNode* parse_relational(Parser* parser);
ASTNode* parse_shift(Parser* parser);
ASTNode* parse_additive(Parser* parser);
ASTNode* parse_multiplicative(Parser* parser);
ASTNode* parse_unary(Parser* parser);
ASTNode* parse_primary(Parser* parser);
void advance(Parser* parser);
bool match(Parser* parser, TokenType type);
void expect(Parser* parser, TokenType type, const char* error_message);

// Инициализация лексера (базовая, без изменений из предыдущего кода)
void init_lexer(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->col = 1;
}

void skip_whitespace(Lexer* lexer) {
    while (lexer->source[lexer->pos]) {
        char c = lexer->source[lexer->pos];
        if (c == ' ' || c == '\t' || c == '\r') {
            lexer->pos++;
            lexer->col++;
        } else if (c == '\n') {
            lexer->pos++;
            lexer->line++;
            lexer->col = 1;
        } else if (c == '/') {
            if (lexer->source[lexer->pos + 1] == '/') {
                while (lexer->source[lexer->pos] && lexer->source[lexer->pos] != '\n')
                    lexer->pos++;
            } else if (lexer->source[lexer->pos + 1] == '*') {
                // Многострочный комментарий
                lexer->pos += 2;
                while (lexer->source[lexer->pos] && 
                       !(lexer->source[lexer->pos] == '*' && lexer->source[lexer->pos + 1] == '/')) {
                    if (lexer->source[lexer->pos] == '\n') {
                        lexer->line++;
                        lexer->col = 1;
                    } else {
                        lexer->col++;
                    }
                    lexer->pos++;
                }
                if (lexer->source[lexer->pos]) {
                    lexer->pos += 2;
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

char* read_number(Lexer* lexer) {
    int start = lexer->pos;
    while (isdigit(lexer->source[lexer->pos]))
        lexer->pos++;
    
    int len = lexer->pos - start;
    char* num = (char*)malloc(len + 1);
    strncpy(num, lexer->source + start, len);
    num[len] = '\0';
    
    lexer->col += len;
    return num;
}

char* read_identifier(Lexer* lexer) {
    int start = lexer->pos;
    while (isalnum(lexer->source[lexer->pos]) || lexer->source[lexer->pos] == '_')
        lexer->pos++;
    
    int len = lexer->pos - start;
    char* id = (char*)malloc(len + 1);
    strncpy(id, lexer->source + start, len);
    id[len] = '\0';
    
    lexer->col += len;
    return id;
}

Token get_next_token(Lexer* lexer) {
    Token token;
    token.value = NULL;
    token.line = lexer->line;
    token.column = lexer->col;
    
    skip_whitespace(lexer);
    
    char c = lexer->source[lexer->pos];
    
    if (c == '\0') {
        token.type = TOKEN_EOF;
        return token;
    }
    
    // Числа
    if (isdigit(c)) {
        token.type = TOKEN_NUMBER;
        token.value = read_number(lexer);
        return token;
    }
    
    // Строки
    if (c == '"') {
        lexer->pos++;
        int start = lexer->pos;
        while (lexer->source[lexer->pos] && lexer->source[lexer->pos] != '"')
            lexer->pos++;
        int len = lexer->pos - start;
        char* str = (char*)malloc(len + 1);
        strncpy(str, lexer->source + start, len);
        str[len] = '\0';
        lexer->pos++;
        token.type = TOKEN_STRING;
        token.value = str;
        return token;
    }
    
    // Идентификаторы и ключевые слова
    if (isalpha(c) || c == '_') {
        char* id = read_identifier(lexer);
        
        int i = 0;
        while (keywords[i].word != NULL) {
            if (strcmp(id, keywords[i].word) == 0) {
                token.type = keywords[i].type;
                free(id);
                return token;
            }
            i++;
        }
        
        token.type = TOKEN_IDENTIFIER;
        token.value = id;
        return token;
    }
    
    // Операторы (многосимвольные проверяем первыми)
    lexer->pos++;
    lexer->col++;
    
    switch (c) {
        case '+':
            if (lexer->source[lexer->pos] == '+') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_PLUS_PLUS;
            } else {
                token.type = TOKEN_PLUS;
            }
            break;
        case '-':
            if (lexer->source[lexer->pos] == '-') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_MINUS_MINUS;
            } else {
                token.type = TOKEN_MINUS;
            }
            break;
        case '*':
            token.type = TOKEN_STAR;
            break;
        case '/':
            token.type = TOKEN_SLASH;
            break;
        case '%':
            token.type = TOKEN_PERCENT;
            break;
        case '=':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_EQ;
            } else {
                token.type = TOKEN_ASSIGN;
            }
            break;
        case '<':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_LE;
            } else if (lexer->source[lexer->pos] == '<') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_SHIFT_LEFT;
            } else {
                token.type = TOKEN_LT;
            }
            break;
        case '>':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_GE;
            } else if (lexer->source[lexer->pos] == '>') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_SHIFT_RIGHT;
            } else {
                token.type = TOKEN_GT;
            }
            break;
        case '!':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_NEQ;
            } else {
                token.type = TOKEN_NOT;
            }
            break;
        case '&':
            if (lexer->source[lexer->pos] == '&') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_AND;
            } else {
                token.type = TOKEN_BIT_AND;
            }
            break;
        case '|':
            if (lexer->source[lexer->pos] == '|') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_OR;
            } else {
                token.type = TOKEN_BIT_OR;
            }
            break;
        case '^':
            token.type = TOKEN_BIT_XOR;
            break;
        case '~':
            token.type = TOKEN_BIT_NOT;
            break;
        case '?':
            token.type = TOKEN_OR; // Временно, нужно добавить TOKEN_QUESTION
            break;
        case ':':
            token.type = TOKEN_ASSIGN; // Временно
            break;
        case ';': token.type = TOKEN_SEMICOLON; break;
        case ',': token.type = TOKEN_COMMA; break;
        case '(': token.type = TOKEN_LPAREN; break;
        case ')': token.type = TOKEN_RPAREN; break;
        case '{': token.type = TOKEN_LBRACE; break;
        case '}': token.type = TOKEN_RBRACE; break;
        case '[': token.type = TOKEN_LBRACKET; break;
        case ']': token.type = TOKEN_RBRACKET; break;
        default:
            printf("Ошибка: неизвестный символ '%c' на строке %d\n", c, lexer->line);
            token.type = TOKEN_EOF;
    }
    
    return token;
}

// Инициализация парсера
void init_parser(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->current = get_next_token(lexer);
}

void advance(Parser* parser) {
    if (parser->current.type != TOKEN_EOF) {
        if (parser->current.value) {
            free(parser->current.value);
        }
        parser->current = get_next_token(parser->lexer);
    }
}

bool match(Parser* parser, TokenType type) {
    if (parser->current.type == type) {
        advance(parser);
        return true;
    }
    return false;
}

void expect(Parser* parser, TokenType type, const char* error_message) {
    if (parser->current.type != type) {
        printf("Ошибка на строке %d, колонке %d: %s\n", 
               parser->current.line, parser->current.column, error_message);
        exit(1);
    }
    advance(parser);
}

// Основная функция парсинга выражений с приоритетами
ASTNode* parse_expression(Parser* parser) {
    return parse_assignment(parser);
}

// Присваивание (самый низкий приоритет)
ASTNode* parse_assignment(Parser* parser) {
    ASTNode* node = parse_ternary(parser);
    
    if (parser->current.type == TOKEN_ASSIGN) {
        advance(parser);
        ASTNode* right = parse_assignment(parser);
        
        // Проверяем, что слева - lvalue (идентификатор)
        if (node->type != NODE_IDENTIFIER) {
            printf("Ошибка: левая часть присваивания должна быть lvalue\n");
            exit(1);
        }
        
        ASTNode* assign = (ASTNode*)malloc(sizeof(ASTNode));
        assign->type = NODE_ASSIGNMENT;
        assign->assignment.var_name = strdup(node->identifier.name);
        assign->assignment.expression = right;
        free_ast(node); // Освобождаем старый узел
        return assign;
    }
    
    return node;
}

// Тернарный оператор ? :
ASTNode* parse_ternary(Parser* parser) {
    ASTNode* node = parse_logical_or(parser);
    
    if (parser->current.type == TOKEN_OR) { // Временно используем TOKEN_OR для '?'
        advance(parser);
        ASTNode* true_expr = parse_expression(parser);
        expect(parser, TOKEN_ASSIGN, "Ожидается ':'"); // Временно
        ASTNode* false_expr = parse_ternary(parser);
        
        ASTNode* ternary = (ASTNode*)malloc(sizeof(ASTNode));
        ternary->type = NODE_TERNARY_OP;
        ternary->ternary.condition = node;
        ternary->ternary.true_expr = true_expr;
        ternary->ternary.false_expr = false_expr;
        return ternary;
    }
    
    return node;
}

// Логическое ИЛИ
ASTNode* parse_logical_or(Parser* parser) {
    ASTNode* node = parse_logical_and(parser);
    
    while (parser->current.type == TOKEN_OR) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_logical_and(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Логическое И
ASTNode* parse_logical_and(Parser* parser) {
    ASTNode* node = parse_bitwise_or(parser);
    
    while (parser->current.type == TOKEN_AND) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_bitwise_or(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Битовое ИЛИ
ASTNode* parse_bitwise_or(Parser* parser) {
    ASTNode* node = parse_bitwise_xor(parser);
    
    while (parser->current.type == TOKEN_BIT_OR) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_bitwise_xor(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Битовое XOR
ASTNode* parse_bitwise_xor(Parser* parser) {
    ASTNode* node = parse_bitwise_and(parser);
    
    while (parser->current.type == TOKEN_BIT_XOR) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_bitwise_and(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Битовое И
ASTNode* parse_bitwise_and(Parser* parser) {
    ASTNode* node = parse_equality(parser);
    
    while (parser->current.type == TOKEN_BIT_AND) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_equality(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Операторы равенства
ASTNode* parse_equality(Parser* parser) {
    ASTNode* node = parse_relational(parser);
    
    while (parser->current.type == TOKEN_EQ || parser->current.type == TOKEN_NEQ) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_relational(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Операторы сравнения
ASTNode* parse_relational(Parser* parser) {
    ASTNode* node = parse_shift(parser);
    
    while (parser->current.type == TOKEN_LT || parser->current.type == TOKEN_GT ||
           parser->current.type == TOKEN_LE || parser->current.type == TOKEN_GE) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_shift(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Операторы сдвига
ASTNode* parse_shift(Parser* parser) {
    ASTNode* node = parse_additive(parser);
    
    while (parser->current.type == TOKEN_SHIFT_LEFT || parser->current.type == TOKEN_SHIFT_RIGHT) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_additive(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Сложение и вычитание
ASTNode* parse_additive(Parser* parser) {
    ASTNode* node = parse_multiplicative(parser);
    
    while (parser->current.type == TOKEN_PLUS || parser->current.type == TOKEN_MINUS) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_multiplicative(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Умножение, деление, остаток
ASTNode* parse_multiplicative(Parser* parser) {
    ASTNode* node = parse_unary(parser);
    
    while (parser->current.type == TOKEN_STAR || parser->current.type == TOKEN_SLASH ||
           parser->current.type == TOKEN_PERCENT) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_unary(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Унарные операторы
ASTNode* parse_unary(Parser* parser) {
    TokenType op = parser->current.type;
    
    if (op == TOKEN_PLUS || op == TOKEN_MINUS || op == TOKEN_NOT || 
        op == TOKEN_BIT_NOT || op == TOKEN_PLUS_PLUS || op == TOKEN_MINUS_MINUS) {
        advance(parser);
        ASTNode* operand = parse_unary(parser);
        
        ASTNode* unary = (ASTNode*)malloc(sizeof(ASTNode));
        unary->type = NODE_UNARY_OP;
        unary->unary.op = op;
        unary->unary.operand = operand;
        return unary;
    }
    
    return parse_primary(parser);
}

// Первичные выражения (литералы, идентификаторы, скобки)
ASTNode* parse_primary(Parser* parser) {
    ASTNode* node = NULL;
    
    if (parser->current.type == TOKEN_NUMBER) {
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_NUMBER;
        node->number.value = atoi(parser->current.value);
        advance(parser);
        return node;
    }
    
    if (parser->current.type == TOKEN_STRING) {
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_STRING;
        node->string.value = strdup(parser->current.value);
        advance(parser);
        return node;
    }
    
    if (parser->current.type == TOKEN_IDENTIFIER) {
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_IDENTIFIER;
        node->identifier.name = strdup(parser->current.value);
        advance(parser);
        
        // Проверка на вызов функции
        if (parser->current.type == TOKEN_LPAREN) {
            advance(parser);
            ASTNode* call = (ASTNode*)malloc(sizeof(ASTNode));
            call->type = NODE_CALL;
            call->call.func_name = strdup(node->identifier.name);
            call->call.args = NULL;
            call->call.arg_count = 0;
            
            // Парсим аргументы
            if (parser->current.type != TOKEN_RPAREN) {
                int capacity = 4;
                call->call.args = (ASTNode**)malloc(capacity * sizeof(ASTNode*));
                do {
                    if (call->call.arg_count >= capacity) {
                        capacity *= 2;
                        call->call.args = (ASTNode**)realloc(call->call.args, capacity * sizeof(ASTNode*));
                    }
                    call->call.args[call->call.arg_count++] = parse_expression(parser);
                } while (match(parser, TOKEN_COMMA));
            }
            
            expect(parser, TOKEN_RPAREN, "Ожидается ')' после аргументов функции");
            free_ast(node);
            return call;
        }
        
        return node;
    }
    
    if (parser->current.type == TOKEN_LPAREN) {
        advance(parser);
        node = parse_expression(parser);
        expect(parser, TOKEN_RPAREN, "Ожидается ')'");
        return node;
    }
    
    printf("Ошибка: неожиданный токен ");
    // print_token(&parser->current);
    printf("\n");
    exit(1);
}

// Функция освобождения AST (рекурсивная)
void free_ast(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_IDENTIFIER:
            free(node->identifier.name);
            break;
        case NODE_STRING:
            free(node->string.value);
            break;
        case NODE_UNARY_OP:
            free_ast(node->unary.operand);
            break;
        case NODE_BINARY_OP:
            free_ast(node->binary.left);
            free_ast(node->binary.right);
            break;
        case NODE_TERNARY_OP:
            free_ast(node->ternary.condition);
            free_ast(node->ternary.true_expr);
            free_ast(node->ternary.false_expr);
            break;
        case NODE_ASSIGNMENT:
            free(node->assignment.var_name);
            free_ast(node->assignment.expression);
            break;
        case NODE_CALL:
            free(node->call.func_name);
            for (int i = 0; i < node->call.arg_count; i++)
                free_ast(node->call.args[i]);
            free(node->call.args);
            break;
        default:
            break;
    }
    
    free(node);
}

// Функция для печати AST (отладка)
void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case NODE_NUMBER:
            printf("NUMBER(%d)\n", node->number.value);
            break;
        case NODE_IDENTIFIER:
            printf("IDENTIFIER(%s)\n", node->identifier.name);
            break;
        case NODE_STRING:
            printf("STRING(\"%s\")\n", node->string.value);
            break;
        case NODE_UNARY_OP:
            printf("UNARY_OP(%d)\n", node->unary.op);
            print_ast(node->unary.operand, indent + 1);
            break;
        case NODE_BINARY_OP:
            printf("BINARY_OP(%d)\n", node->binary.op);
            print_ast(node->binary.left, indent + 1);
            print_ast(node->binary.right, indent + 1);
            break;
        case NODE_TERNARY_OP:
            printf("TERNARY_OP\n");
            printf("  Condition:\n");
            print_ast(node->ternary.condition, indent + 1);
            printf("  True:\n");
            print_ast(node->ternary.true_expr, indent + 1);
            printf("  False:\n");
            print_ast(node->ternary.false_expr, indent + 1);
            break;
        case NODE_ASSIGNMENT:
            printf("ASSIGNMENT(%s)\n", node->assignment.var_name);
            print_ast(node->assignment.expression, indent + 1);
            break;
        case NODE_CALL:
            printf("CALL(%s, %d args)\n", node->call.func_name, node->call.arg_count);
            for (int i = 0; i < node->call.arg_count; i++)
                print_ast(node->call.args[i], indent + 1);
            break;
        default:
            printf("UNKNOWN\n");
    }
}

// Тестовая функция
int main() {
    // Тестовые выражения
    const char* expressions[] = {
        "1 + 2 * 3",
        "(1 + 2) * 3",
        "a = b = 5",
        "x = 10 + (y = 20) * 3",
        "a + b * c - d / e % f",
        "x > 0 && y < 10 || z == 5",
        "!a && b || c",
        "a << 2 + b >> 1",
        "max(10, 20, 30)",
        "a ? b : c",
        "++i",
        "-x * 3",
        NULL
    };
    
    for (int i = 0; expressions[i] != NULL; i++) {
        printf("\n=== Тест %d: %s ===\n", i+1, expressions[i]);
        
        Lexer lexer;
        init_lexer(&lexer, expressions[i]);
        
        Parser parser;
        init_parser(&parser, &lexer);
        
        ASTNode* ast = parse_expression(&parser);
        
        printf("AST:\n");
        print_ast(ast, 0);
        
        free_ast(ast);
    }
    
    return 0;
}

////##Теперь добавим интерпретатор для вычисления выражений:

// Символьная таблица
typedef struct {
    char* name;
    int value;
} Symbol;

Symbol sym_table[100];
int sym_count = 0;

int get_variable(const char* name) {
    for (int i = 0; i < sym_count; i++) {
        if (strcmp(sym_table[i].name, name) == 0)
            return sym_table[i].value;
    }
    return 0;
}

void set_variable(const char* name, int value) {
    for (int i = 0; i < sym_count; i++) {
        if (strcmp(sym_table[i].name, name) == 0) {
            sym_table[i].value = value;
            return;
        }
    }
    sym_table[sym_count].name = strdup(name);
    sym_table[sym_count].value = value;
    sym_count++;
}

// Интерпретация AST
int interpret_ast(ASTNode* node) {
    if (!node) return 0;
    
    switch (node->type) {
        case NODE_NUMBER:
            return node->number.value;
            
        case NODE_IDENTIFIER:
            return get_variable(node->identifier.name);
            
        case NODE_STRING:
            printf("%s", node->string.value);
            return 0;
            
        case NODE_UNARY_OP: {
            int val = interpret_ast(node->unary.operand);
            switch (node->unary.op) {
                case TOKEN_PLUS: return +val;
                case TOKEN_MINUS: return -val;
                case TOKEN_NOT: return !val;
                case TOKEN_BIT_NOT: return ~val;
                case TOKEN_PLUS_PLUS: return val + 1;
                case TOKEN_MINUS_MINUS: return val - 1;
                default: return 0;
            }
        }
        
        case NODE_BINARY_OP: {
            int left = interpret_ast(node->binary.left);
            int right = interpret_ast(node->binary.right);
            
            switch (node->binary.op) {
                case TOKEN_PLUS: return left + right;
                case TOKEN_MINUS: return left - right;
                case TOKEN_STAR: return left * right;
                case TOKEN_SLASH: return right != 0 ? left / right : 0;
                case TOKEN_PERCENT: return right != 0 ? left % right : 0;
                case TOKEN_LT: return left < right;
                case TOKEN_GT: return left > right;
                case TOKEN_LE: return left <= right;
                case TOKEN_GE: return left >= right;
                case TOKEN_EQ: return left == right;
                case TOKEN_NEQ: return left != right;
                case TOKEN_AND: return left && right;
                case TOKEN_OR: return left || right;
                case TOKEN_BIT_AND: return left & right;
                case TOKEN_BIT_OR: return left | right;
                case TOKEN_BIT_XOR: return left ^ right;
                case TOKEN_SHIFT_LEFT: return left << right;
                case TOKEN_SHIFT_RIGHT: return left >> right;
                default: return 0;
            }
        }
        
        case NODE_TERNARY_OP:
            return interpret_ast(node->ternary.condition) ? 
                   interpret_ast(node->ternary.true_expr) : 
                   interpret_ast(node->ternary.false_expr);
        
        case NODE_ASSIGNMENT:
            set_variable(node->assignment.var_name, 
                        interpret_ast(node->assignment.expression));
            return get_variable(node->assignment.var_name);
        
        case NODE_CALL: {
            if (strcmp(node->call.func_name, "print") == 0) {
                for (int i = 0; i < node->call.arg_count; i++) {
                    int val = interpret_ast(node->call.args[i]);
                    printf("%d", val);
                    if (i < node->call.arg_count - 1) printf(" ");
                }
                printf("\n");
                return 0;
            }
            printf("Неизвестная функция: %s\n", node->call.func_name);
            return 0;
        }
        
        default:
            printf("Неизвестный тип узла\n");
            return 0;
    }
}

// Тест интерпретатора
int main() {
    const char* test_code = 
        "a = 10\n"
        "b = 20\n"
        "c = a + b * 2\n"
        "print(c)\n"
        "d = (c - 10) / 2\n"
        "print(d)\n"
        "e = a > b ? a : b\n"
        "print(e)\n"
        "f = -a + b\n"
        "print(f)\n";
    
    printf("=== Интерпретация программы ===\n");
    
    // Разбиваем на строки и интерпретируем
    char* code_copy = strdup(test_code);
    char* line = strtok(code_copy, "\n");
    
    while (line) {
        printf("> %s\n", line);
        
        Lexer lexer;
        init_lexer(&lexer, line);
        
        Parser parser;
        init_parser(&parser, &lexer);
        
        ASTNode* ast = parse_expression(&parser);
        int result = interpret_ast(ast);
        
        // Если выражение не было присваиванием или вызовом функции
        if (ast->type != NODE_ASSIGNMENT && ast->type != NODE_CALL) {
            printf("= %d\n", result);
        }
        
        free_ast(ast);
        line = strtok(NULL, "\n");
    }
    
    free(code_copy);
    
    return 0;
}