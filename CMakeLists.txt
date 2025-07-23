cmake_minimum_required(VERSION 3.16)

# Force use LLVM 20 compilers if LLVM20_HOME is set
if(DEFINED ENV{LLVM20_HOME})
    set(CMAKE_C_COMPILER "$ENV{LLVM20_HOME}/bin/clang")
    set(CMAKE_CXX_COMPILER "$ENV{LLVM20_HOME}/bin/clang++")
    message(STATUS "Using LLVM 20 compilers from: $ENV{LLVM20_HOME}")
endif()

project(MiniSwift 
    VERSION 1.0.0
    DESCRIPTION "A Swift-like programming language interpreter"
    LANGUAGES C CXX
)

# Include ExternalProject module for third-party dependencies
include(ExternalProject)

# Detect system architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(TARGET_ARCH "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(TARGET_ARCH "x86_64")
else()
    set(TARGET_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
endif()

message(STATUS "Target architecture: ${TARGET_ARCH}")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
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

set(SEMANTIC_SOURCES
    src/semantic/SemanticAnalyzer.cpp
    src/semantic/SymbolTable.cpp
    src/semantic/TypeSystem.cpp
    src/semantic/TypedAST.cpp
)

set(CODEGEN_SOURCES
    src/codegen/LLVMCodeGenerator.cpp
)

set(MAIN_SOURCES
    src/main.cpp
    src/jit_debug_stubs.cpp
)

# Third-party dependencies configuration
set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third")
set(THIRD_PARTY_INSTALL_DIR "${THIRD_PARTY_DIR}/install")

# 使用跨平台的zstd构建脚本
add_subdirectory(third/zstd)

# zstd变量已经在子目录中设置，这里直接使用
# ZSTD_LIBRARY 和 ZSTD_TARGET 变量已经可用
set(ZSTD_INCLUDE_DIR "${THIRD_PARTY_INSTALL_DIR}/include")

# Use LLVM from environment variable LLVM20_HOME
if(DEFINED ENV{LLVM20_HOME})
    set(LLVM_ROOT "$ENV{LLVM20_HOME}")
    set(LLVM_DIR "${LLVM_ROOT}/lib/cmake/llvm")
    message(STATUS "Using LLVM from LLVM20_HOME: ${LLVM_ROOT}")
    
    # Use system compilers but LLVM 20 headers and libraries
# This avoids C++ runtime linking issues while using LLVM 20
include_directories("${LLVM_ROOT}/include")
include_directories("${ZSTD_INCLUDE_DIR}")

# Use basic LLVM flags but enable RTTI for our code and disable JIT debugging
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -stdlib=libc++ -frtti -arch ${TARGET_ARCH} -DLLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1 -DLLVM_ENABLE_ZSTD=0")

# 获取LLVM编译标志并过滤（参考成功的build_with_llvm20.sh脚本）
execute_process(
    COMMAND ${LLVM_ROOT}/bin/llvm-config --cxxflags
    OUTPUT_VARIABLE LLVM_CXXFLAGS_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 过滤LLVM标志，移除禁用异常和RTTI的标志
string(REGEX REPLACE "-fno-exceptions" "" LLVM_CXXFLAGS_FILTERED "${LLVM_CXXFLAGS_RAW}")
string(REGEX REPLACE "-fno-rtti" "" LLVM_CXXFLAGS "${LLVM_CXXFLAGS_FILTERED}")

# 获取LLVM链接标志并过滤
execute_process(
    COMMAND ${LLVM_ROOT}/bin/llvm-config --ldflags
    OUTPUT_VARIABLE LLVM_LDFLAGS_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 过滤链接标志，移除有问题的库路径
string(REGEX REPLACE "/opt/homebrew/lib/libzstd\.a" "" LLVM_LDFLAGS_FILTERED "${LLVM_LDFLAGS_RAW}")
string(REGEX REPLACE "-L/opt/homebrew/lib" "" LLVM_LDFLAGS "${LLVM_LDFLAGS_FILTERED}")

# Force target architecture to avoid conflicts
set(CMAKE_OSX_ARCHITECTURES "${TARGET_ARCH}")

# 使用llvm-config获取特定的LLVM库组合（参考成功的build_with_llvm20.sh脚本）
execute_process(
    COMMAND ${LLVM_ROOT}/bin/llvm-config --libs core support irreader executionengine mcjit native
    OUTPUT_VARIABLE LLVM_LIBS_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 获取系统库
execute_process(
    COMMAND ${LLVM_ROOT}/bin/llvm-config --system-libs
    OUTPUT_VARIABLE LLVM_SYSTEM_LIBS_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 过滤系统库，移除有问题的libzstd.a路径
string(REGEX REPLACE "/opt/homebrew/lib/libzstd\.a" "" LLVM_SYSTEM_LIBS "${LLVM_SYSTEM_LIBS_RAW}")

# 将LLVM库字符串转换为列表
separate_arguments(LLVM_LIBRARIES UNIX_COMMAND "${LLVM_LIBS_RAW}")
separate_arguments(LLVM_SYSTEM_LIBRARIES UNIX_COMMAND "${LLVM_SYSTEM_LIBS}")

# 添加我们自己的zstd库
set(MINIMAL_LLVM_LIBS ${LLVM_LIBRARIES} ${LLVM_SYSTEM_LIBRARIES} "${ZSTD_LIBRARY}")

message(STATUS "Using minimal LLVM libraries for basic functionality")
    
else()
    message(FATAL_ERROR "LLVM20_HOME environment variable is not set. Please set it to your LLVM 20 installation directory.")
endif()

find_package(LLVM REQUIRED CONFIG PATHS ${LLVM_DIR} NO_DEFAULT_PATH)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Using CXX compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "LLVM include directories: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")

# Set up LLVM
include_directories(${LLVM_INCLUDE_DIRS})
include_directories("${LLVM_ROOT}/include")
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Add LLVM-specific compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXXFLAGS}")

# Find the libraries that correspond to the LLVM components
llvm_map_components_to_libnames(llvm_libs support core irreader executionengine interpreter mcjit nativecodegen)

# Combine all sources
set(ALL_SOURCES
    ${MAIN_SOURCES}
    ${LEXER_SOURCES}
    ${PARSER_SOURCES}
    ${INTERPRETER_SOURCES}
    ${OOP_SOURCES}
    ${MEMORY_SOURCES}
    ${SEMANTIC_SOURCES}
    ${CODEGEN_SOURCES}
)

# Create the executable
add_executable(miniswift ${ALL_SOURCES})

# Add dependency on zstd (现在通过ZSTD_TARGET处理)
if(TARGET ${ZSTD_TARGET})
    add_dependencies(miniswift ${ZSTD_TARGET})
endif()

# 应用LLVM编译标志
target_compile_options(miniswift PRIVATE ${LLVM_CXXFLAGS})

# 应用LLVM链接标志（正确分离参数）
separate_arguments(LLVM_LDFLAGS_LIST UNIX_COMMAND "${LLVM_LDFLAGS}")
target_link_options(miniswift PRIVATE ${LLVM_LDFLAGS_LIST})

# 添加必要的系统库（参考build_with_llvm20.sh脚本）
find_program(XCRUN_EXECUTABLE xcrun)
if(XCRUN_EXECUTABLE)
    execute_process(
        COMMAND ${XCRUN_EXECUTABLE} --show-sdk-path
        OUTPUT_VARIABLE SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    target_link_options(miniswift PRIVATE "-L${SDK_PATH}/usr/lib")
endif()

# 添加LLVM的libunwind库和C++运行时库
target_link_libraries(miniswift PRIVATE "${LLVM_ROOT}/lib/libunwind.a")
# 使用LLVM_ROOT中的C++标准库和异常处理库，避免与系统库冲突
target_link_libraries(miniswift PRIVATE "${LLVM_ROOT}/lib/libc++.a" "${LLVM_ROOT}/lib/libc++abi.a")
# 设置RPATH以便找到LLVM和系统库
set_target_properties(miniswift PROPERTIES
    BUILD_RPATH "${LLVM_ROOT}/lib"
    INSTALL_RPATH "${LLVM_ROOT}/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
    MACOSX_RPATH TRUE
)

# Link libraries with LLVM 20
target_link_libraries(miniswift PRIVATE ${MINIMAL_LLVM_LIBS})

# Add additional system libraries for macOS
if(APPLE)
    target_link_libraries(miniswift PRIVATE "-framework CoreFoundation" "-framework Security" "-lz")
endif()

# Set target properties
set_target_properties(miniswift PROPERTIES
    OUTPUT_NAME "miniswift"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")