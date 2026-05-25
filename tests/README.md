# Unit Testing с Unity

Этот проект использует фреймворк **Unity** для unit-тестирования на C.

## Запуск тестов

### В терминале:
```bash
cd build
cmake -G Ninja ..
ninja lexer_tests
./bin/lexer_tests.exe
```

### Через CTest:
```bash
cd build
ctest --output-on-failure
```

## Добавление новых тестов

1. Создайте файл `tests/your_tests.c`
2. Включите `#include "unity.h"` 
3. Добавьте функции тестов с именем `test_название_теста(void)`
4. Добавьте файл в `CMakeLists.txt`:
   ```cmake
   add_executable(your_tests 
       tests/unity.c
       tests/your_tests.c
   )
   target_include_directories(your_tests PRIVATE tests include ${CMAKE_CURRENT_SOURCE_DIR})
   add_test(NAME YourTests COMMAND your_tests)
   ```

## Команды тестов Unity

- `TEST_ASSERT_EQUAL(expected, actual)` - сравнение значений
- `TEST_ASSERT_EQUAL_STRING(s1, s2)` - сравнение строк
- `TEST_ASSERT_NULL(ptr)` - проверка на NULL
- `TEST_ASSERT_NOT_NULL(ptr)` - проверка не на NULL
- `TEST_ASSERT(condition)` - общая проверка условия

## Пример теста

```c
#include "unity.h"
#include "include/file.h"

void test_add_numbers(void) {
    int result = add(2, 3);
    TEST_ASSERT_EQUAL(5, result);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_add_numbers);
    return UNITY_END();
}
```
