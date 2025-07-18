cmake_minimum_required(VERSION 3.10)

project(MiniSwift CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(miniswift src/main.cpp src/lexer/Lexer.cpp src/parser/Parser.cpp src/parser/AstPrinter.cpp src/interpreter/Interpreter.cpp
    src/interpreter/Environment.cpp src/interpreter/Value.cpp src/interpreter/OOP/Property.cpp 
    src/interpreter/OOP/Method.cpp src/interpreter/OOP/MethodInterpreter.cpp)