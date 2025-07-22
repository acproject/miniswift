cmake_minimum_required(VERSION 3.16)

project(MiniSwift 
    VERSION 1.0.0
    DESCRIPTION "A Swift-like programming language interpreter"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# Include directories
include_directories(
    src
)

# Source files organized by component
set(LEXER_SOURCES
    src/lexer/Lexer.cpp
)

set(PARSER_SOURCES
    src/parser/Parser.cpp
    src/parser/AstPrinter.cpp
    src/parser/AccessControl.cpp
    src/parser/ErrorNodes.cpp
)

set(INTERPRETER_SOURCES
    src/interpreter/Interpreter.cpp
    src/interpreter/Environment.cpp
    src/interpreter/Value.cpp
)

set(OOP_SOURCES
    src/interpreter/OOP/Property.cpp
    src/interpreter/OOP/Method.cpp
    src/interpreter/OOP/MethodInterpreter.cpp
    src/interpreter/OOP/Constructor.cpp
    src/interpreter/OOP/Inheritance.cpp
    src/interpreter/OOP/Subscript.cpp
    src/interpreter/OOP/Optional.cpp
)

set(MEMORY_SOURCES
    src/interpreter/Memory/ARC.cpp
    src/interpreter/Memory/MemorySafety.cpp
    src/interpreter/Memory/MemoryManager.cpp
)

set(MAIN_SOURCES
    src/main.cpp
)

# Combine all sources
set(ALL_SOURCES
    ${MAIN_SOURCES}
    ${LEXER_SOURCES}
    ${PARSER_SOURCES}
    ${INTERPRETER_SOURCES}
    ${OOP_SOURCES}
    ${MEMORY_SOURCES}
)

# Create the executable
add_executable(miniswift ${ALL_SOURCES})

# Set target properties
set_target_properties(miniswift PROPERTIES
    OUTPUT_NAME "miniswift"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")