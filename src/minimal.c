#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_VARS 100
#define MAX_VAR_NAME 32
#define MAX_CODE 1024

// Представление переменной
typedef struct {
    char name[MAX_VAR_NAME];
    int value;
} Variable;

Variable vars[MAX_VARS];
int var_count = 0;

int get_var(const char *name) {
    for (int i = 0; i < var_count; i++)
        if (strcmp(vars[i].name, name) == 0)
            return vars[i].value;
    return 0; // не найдена -> 0
}

void set_var(const char *name, int value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            vars[i].value = value;
            return;
        }
    }
    if (var_count < MAX_VARS) {
        strncpy(vars[var_count].name, name, MAX_VAR_NAME);
        vars[var_count].value = value;
        var_count++;
    }
}

// Упрощённый парсер выражения (только + - * / и целые числа/переменные)
int eval_expr(const char **p) {
    // Пропуск пробелов
    while (isspace(**p)) (*p)++;
    
    int left;
    if (isdigit(**p)) {
        left = strtol(*p, (char**)p, 10);
    } else if (isalpha(**p)) {
        char vname[MAX_VAR_NAME];
        int i = 0;
        while (isalnum(**p) && i < MAX_VAR_NAME-1) vname[i++] = *(*p)++;
        vname[i] = '\0';
        left = get_var(vname);
    } else {
        left = 0;
    }
    
    while (1) {
        while (isspace(**p)) (*p)++;
        char op = **p;
        if (op != '+' && op != '-' && op != '*' && op != '/') break;
        (*p)++;
        while (isspace(**p)) (*p)++;
        
        int right;
        if (isdigit(**p)) {
            right = strtol(*p, (char**)p, 10);
        } else if (isalpha(**p)) {
            char vname[MAX_VAR_NAME];
            int i = 0;
            while (isalnum(**p) && i < MAX_VAR_NAME-1) vname[i++] = *(*p)++;
            vname[i] = '\0';
            right = get_var(vname);
        } else {
            right = 0;
        }
        
        switch (op) {
            case '+': left += right; break;
            case '-': left -= right; break;
            case '*': left *= right; break;
            case '/': if (right != 0) left /= right; break;
        }
    }
    return left;
}

// Интерпретатор одной строки
int interpret_line(const char *line) {
    const char *p = line;
    
    // Пропуск пробелов
    while (isspace(*p)) p++;
    if (*p == '\0') return 1; // пустая строка
    
    // Обработка оператора вывода (print)
    if (strncmp(p, "print", 5) == 0 && isspace(p[5])) {
        p += 5;
        int val = eval_expr(&p);
        printf("%d\n", val);
        return 1;
    }
    
    // Обработка оператора присваивания (var = expr)
    if (isalpha(*p)) {
        char vname[MAX_VAR_NAME];
        int i = 0;
        while (isalnum(*p) && i < MAX_VAR_NAME-1) vname[i++] = *p++;
        vname[i] = '\0';
        
        // Пропуск пробелов и проверка '='
        while (isspace(*p)) p++;
        if (*p == '=') {
            p++;
            int value = eval_expr(&p);
            set_var(vname, value);
            return 1;
        }
    }
    
    // Если дошли до сюда — неизвестная команда
    printf("Unknown statement: %s\n", line);
    return 0;
}

// Загрузка и выполнение программы построчно
void run_program(const char *program) {
    char buffer[MAX_CODE];
    strncpy(buffer, program, MAX_CODE);
    buffer[MAX_CODE-1] = '\0';
    
    char *line = strtok(buffer, "\n");
    while (line) {
        interpret_line(line);
        line = strtok(NULL, "\n");
    }
}

// Пример программы на упрощённом C
int main() {
    const char *program =
        "a = 10\n"
        "b = 20\n"
        "c = a + b * 2\n"
        "print c\n"
        "d = (c - 10) / 2   # скобки не поддерживаются в этом примере\n"
        "print d\n";
    
    printf("Running simplified C interpreter:\n");
    run_program(program);
    
    return 0;
}