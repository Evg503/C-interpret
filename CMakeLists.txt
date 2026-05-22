cmake_minimum_required(VERSION 3.10)
project(CSimpleInterpreter C)

# Устанавливаем стандарт языка C
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)


# Определяем исходные файлы
set(SOURCES
    src/test_main.c
    src/lexer.c
    src/parser.c
    src/interpreter.c

)

set(HEADERS
    include/lexer.h
    include/parser.h
    include/interpreter.h
)

include_directories(include) 

# Создаем исполняемый файл
add_executable(c_interpreter ${SOURCES} ${HEADERS})
add_executable(c_minimal src/minimal.c)

# Windows-specific настройки
if(WIN32)
    # Явно указываем использование консольного приложения
    # set_target_properties(c_interpreter PROPERTIES
    #     LINK_FLAGS #"/SUBSYSTEM:CONSOLE"
    # )
    
    # Если используете LLVM/Clang с lld-link
    if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
	# Опции сборки
	set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra  -Wno-deprecated-declarations")
	set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

        target_link_options(c_interpreter PRIVATE 
            -fuse-ld=lld
            #-lc
            -lmsvcrt
        )
        target_link_options(c_minimal PRIVATE 
            -fuse-ld=lld
            #-lc
            -lmsvcrt
        )
    endif()
endif()

# Настройка путей для заголовочных файлов (если понадобятся)
# target_include_directories(c_interpreter PRIVATE include)

# Настройка линковки (если потребуются дополнительные библиотеки)
# target_link_libraries(c_interpreter m)  # для математической библиотеки

# Устанавливаем свойства выходного файла
set_target_properties(c_interpreter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_target_properties(c_minimal PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Опционально: добавить поддержку санитайзеров для отладки
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # target_compile_options(c_interpreter PRIVATE -fsanitize=address -g)
    # target_link_options(c_interpreter PRIVATE -fsanitize=address)
endif()

# Выводим информацию о сборке
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
