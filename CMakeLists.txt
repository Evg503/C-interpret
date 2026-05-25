cmake_minimum_required(VERSION 3.10)
project(CSimpleInterpreter C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include) 

add_executable(c_interpreter 
    src/test_main.c
    src/lexer.c
    src/parser.c
    src/interpreter.c
)
add_executable(c_minimal src/minimal.c)

# Unity tests
add_executable(lexer_tests 
    tests/unity.c
    tests/lexer_tests.c
)
target_include_directories(lexer_tests PRIVATE tests include ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(c_interpreter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_target_properties(c_minimal PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_target_properties(lexer_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_test(NAME LexerTests COMMAND lexer_tests)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
