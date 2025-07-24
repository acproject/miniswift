cmake_minimum_required(VERSION 3.16)

# Use system default compilers (will be compatible with system LLVM)

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
    add_compile_options(/W4 /EHsc)
    # Enable RTTI for MSVC
    add_compile_options(/GR)
    # Disable specific warnings for LLVM compatibility
    add_compile_options(/wd4244 /wd4267 /wd4996)
    # Set runtime library to static
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
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

# Find LLVM using standard CMake find_package
find_package(LLVM REQUIRED CONFIG)

# Include zstd directories
include_directories("${ZSTD_INCLUDE_DIR}")

# Platform-specific LLVM flags
if(WIN32)
    # Windows-specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DLLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1")
    # Set target architecture for Windows
    if(CMAKE_GENERATOR_PLATFORM)
        # Use the platform specified by the generator
    elseif(TARGET_ARCH STREQUAL "x86_64")
        set(CMAKE_GENERATOR_PLATFORM "x64")
    else()
        set(CMAKE_GENERATOR_PLATFORM "Win32")
    endif()
else()
    # Unix-like systems (macOS, Linux)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -DLLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1")
    # Set architecture for macOS
    if(APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch ${TARGET_ARCH}")
        set(CMAKE_OSX_ARCHITECTURES "${TARGET_ARCH}")
    endif()
endif()

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Using CXX compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "LLVM include directories: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")

# Set up LLVM
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Find the libraries that correspond to the LLVM components
# Include target-specific components for the current architecture
if(TARGET_ARCH STREQUAL "arm64")
    llvm_map_components_to_libnames(llvm_libs support core irreader executionengine interpreter mcjit nativecodegen AArch64 AArch64AsmParser AArch64CodeGen AArch64Desc AArch64Disassembler AArch64Info)
elseif(WIN32)
    # Windows typically uses x86_64, but also include additional Windows-specific components
    llvm_map_components_to_libnames(llvm_libs support core irreader executionengine interpreter mcjit nativecodegen X86 X86AsmParser X86CodeGen X86Desc X86Disassembler X86Info)
else()
    # Other Unix-like systems (Linux, etc.)
    llvm_map_components_to_libnames(llvm_libs support core irreader executionengine interpreter mcjit nativecodegen X86 X86AsmParser X86CodeGen X86Desc X86Disassembler X86Info)
endif()

# Combine LLVM libraries with our zstd library
set(MINIMAL_LLVM_LIBS ${llvm_libs} "${ZSTD_LIBRARY}")

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

# Add dependency on zstd
if(TARGET ${ZSTD_TARGET})
    add_dependencies(miniswift ${ZSTD_TARGET})
endif()

# Link with LLVM libraries
if(WIN32)
message("Do noting")
else ()
    target_link_libraries(miniswift PRIVATE ${MINIMAL_LLVM_LIBS})
endif ()
# Add platform-specific system libraries
if(APPLE)
    # macOS frameworks and libraries
    target_link_libraries(miniswift PRIVATE "-framework CoreFoundation" "-framework Security" "-lz")
elseif(WIN32)
    # Windows system libraries
    target_link_libraries(miniswift PRIVATE 
        kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32
        ws2_32 dbghelp psapi version
    )
else()
    # Linux and other Unix-like systems
    target_link_libraries(miniswift PRIVATE pthread dl m)
endif()

# Set target properties
if(WIN32)
    set_target_properties(miniswift PROPERTIES
        OUTPUT_NAME "miniswift"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        # Windows-specific properties
        WIN32_EXECUTABLE FALSE
        # Set subsystem to console
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
else()
    set_target_properties(miniswift PROPERTIES
        OUTPUT_NAME "miniswift"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
endif()

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")