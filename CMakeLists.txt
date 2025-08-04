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
# 禁用LTO以避免与LLVM 20.1.0的兼容性问题
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)

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
    src/interpreter/ErrorHandling.cpp
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

set(CONCURRENCY_SOURCES
    src/interpreter/Concurrency/ConcurrencyRuntime.cpp
)

# IO and Network modules
set(IO_SOURCES
    src/interpreter/IO/IORuntime.cpp
    src/interpreter/IONetworkIntegration.cpp
)

set(NETWORK_SOURCES
    src/interpreter/Network/NetworkRuntime.cpp
)

# UI module sources
set(UI_SOURCES
    src/ui/UIRuntime.cpp
    src/ui/GTK4Backend.cpp
    src/ui/UIIntegration.cpp
    src/ui/TextWidget.cpp
    src/ui/ButtonWidget.cpp
    src/ui/VStackWidget.cpp
    src/ui/HStackWidget.cpp
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

# Include zstd directories
include_directories("${ZSTD_INCLUDE_DIR}")

message(STATUS "Using C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Using CXX compiler: ${CMAKE_CXX_COMPILER}")

# Find LLVM - 在macOS上优先使用Homebrew LLVM
if(APPLE)
    # 尝试使用MacPorts LLVM
    execute_process(
        COMMAND port installed | grep llvm
        OUTPUT_VARIABLE PORT_LLVM_OUTPUT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    if(PORT_LLVM_OUTPUT)
        # 检查常见的MacPorts LLVM路径
        set(POSSIBLE_LLVM_PATHS 
            "/opt/local/libexec/llvm-18"
            "/opt/local/libexec/llvm-17"
            "/opt/local/libexec/llvm-16"
            "/opt/local/libexec/llvm-15"
        )
        
        foreach(LLVM_PATH ${POSSIBLE_LLVM_PATHS})
            if(EXISTS "${LLVM_PATH}")
                message(STATUS "Found MacPorts LLVM at: ${LLVM_PATH}")
                set(CMAKE_PREFIX_PATH "${LLVM_PATH}" ${CMAKE_PREFIX_PATH})
                set(LLVM_DIR "${LLVM_PATH}/lib/cmake/llvm")
                set(MACPORTS_LLVM_PREFIX "${LLVM_PATH}")
                break()
            endif()
        endforeach()
    else()
        # 回退到Homebrew LLVM
        execute_process(
            COMMAND brew --prefix llvm
            OUTPUT_VARIABLE HOMEBREW_LLVM_PREFIX
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        if(HOMEBREW_LLVM_PREFIX AND EXISTS "${HOMEBREW_LLVM_PREFIX}")
            message(STATUS "Found Homebrew LLVM at: ${HOMEBREW_LLVM_PREFIX}")
            set(CMAKE_PREFIX_PATH "${HOMEBREW_LLVM_PREFIX}" ${CMAKE_PREFIX_PATH})
            set(LLVM_DIR "${HOMEBREW_LLVM_PREFIX}/lib/cmake/llvm")
        endif()
    endif()
endif()

find_package(LLVM CONFIG)

if(LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
    
    # Set up LLVM
    include_directories(${LLVM_INCLUDE_DIRS})
    separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
    add_definitions(${LLVM_DEFINITIONS_LIST})
    
    # Add LLVM library directories
    link_directories(${LLVM_LIBRARY_DIRS})
else()
    message(WARNING "LLVM not found, some features may be disabled")
    set(LLVM_INCLUDE_DIRS "")
    set(LLVM_DEFINITIONS_LIST "")
    set(LLVM_LIBRARY_DIRS "")
endif()

# Find the libraries that correspond to the LLVM components
# 根据操作系统选择不同的LLVM组件
if(LLVM_FOUND)
    if(APPLE)
    # macOS: 使用LLVM组件映射，添加AArch64支持
    llvm_map_components_to_libnames(llvm_libs 
        support core irreader executionengine interpreter mc mcjit bitwriter 
        x86codegen x86asmparser x86desc x86disassembler x86info 
        aarch64codegen aarch64asmparser aarch64desc aarch64disassembler aarch64info 
        target orcjit orctargetprocess analysis transformutils scalaropts 
        instcombine vectorize ipo passes codegen asmprinter selectiondag)
    message(STATUS "Using LLVM component mapping for macOS")
    
    # 添加LLVM系统依赖库
    if(MACPORTS_LLVM_PREFIX)
        execute_process(
            COMMAND ${MACPORTS_LLVM_PREFIX}/bin/llvm-config --system-libs
            OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    elseif(HOMEBREW_LLVM_PREFIX)
        execute_process(
            COMMAND ${HOMEBREW_LLVM_PREFIX}/bin/llvm-config --system-libs
            OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()
    
    if(LLVM_SYSTEM_LIBS)
        string(REPLACE " " ";" LLVM_SYSTEM_LIBS_LIST "${LLVM_SYSTEM_LIBS}")
        list(APPEND llvm_libs ${LLVM_SYSTEM_LIBS_LIST})
        message(STATUS "Added LLVM system libs: ${LLVM_SYSTEM_LIBS}")
    endif()
else()
    # Linux和Windows使用标准映射
    llvm_map_components_to_libnames(llvm_libs support core irreader executionengine interpreter mc mcjit bitwriter x86codegen x86asmparser x86desc x86disassembler x86info target orcjit orctargetprocess)
    endif()
    
    message(STATUS "Building with LLVM code generation enabled")
    message(STATUS "LLVM libraries: ${llvm_libs}")
else()
    # LLVM not found, set empty library list
    set(llvm_libs "")
    message(STATUS "Building without LLVM code generation")
endif()

# Set up basic compiler flags
if(WIN32)
    # Windows-specific flags
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    endif()
else()
    # Unix-like systems (macOS, Linux)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")
    # Add pthread support for concurrency
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# Use zstd and LLVM libraries
set(ALL_LIBS ${ZSTD_LIBRARY} ${llvm_libs})

# Check if vcpkg GTK4 is available
if(EXISTS "D:/vcpkg/installed/x64-windows")
    set(GTK4_FOUND TRUE)
    set(GTK4_VERSION "4.16.3")
    message(STATUS "GTK4 found via vcpkg at D:/vcpkg/installed/x64-windows")
    
    # Set GTK4 include directories
    set(GTK4_INCLUDE_DIRS
        "D:/vcpkg/installed/x64-windows/include/gtk-4.0"
        "D:/vcpkg/installed/x64-windows/include/glib-2.0"
        "D:/vcpkg/installed/x64-windows/lib/glib-2.0/include"
        "D:/vcpkg/installed/x64-windows/include/pango-1.0"
        "D:/vcpkg/installed/x64-windows/include/harfbuzz"
        "D:/vcpkg/installed/x64-windows/include/cairo"
        "D:/vcpkg/installed/x64-windows/include/gdk-pixbuf-2.0"
        "D:/vcpkg/installed/x64-windows/include/graphene-1.0"
        "D:/vcpkg/installed/x64-windows/lib/graphene-1.0/include"
    )
    
    # Set GTK4 library directories and libraries
    set(GTK4_LIBRARY_DIRS "D:/vcpkg/installed/x64-windows/lib")
    set(GTK4_LIBRARIES
        gtk-4 glib-2.0 gobject-2.0 gio-2.0 pango-1.0 pangocairo-1.0
        cairo cairo-gobject gdk_pixbuf-2.0 harfbuzz graphene-1.0
    )
else()
    set(GTK4_FOUND FALSE)
    message(STATUS "GTK4 not found - vcpkg installation not detected")
endif()



# =============================================================================
# Dynamic Libraries - Create libraries first before main executable
# =============================================================================

# Core library sources - combining interpreter, lexer, parser, runtime
set(CORE_SOURCES
    # Runtime
    src/runtime/runtime_interface.cpp
    
    # Lexer
    src/lexer/Lexer.cpp
    
    # Parser
    src/parser/Parser.cpp
    src/parser/AstPrinter.cpp
    src/parser/AccessControl.cpp
    src/parser/ErrorNodes.cpp
    
    # Interpreter
    src/interpreter/Interpreter.cpp
    src/interpreter/Environment.cpp
    src/interpreter/Value.cpp
    src/interpreter/ErrorHandling.cpp
    
    # OOP
    src/interpreter/OOP/Property.cpp
    src/interpreter/OOP/Method.cpp
    src/interpreter/OOP/MethodInterpreter.cpp
    src/interpreter/OOP/Constructor.cpp
    src/interpreter/OOP/Inheritance.cpp
    src/interpreter/OOP/Subscript.cpp
    src/interpreter/OOP/Optional.cpp
    
    # Memory
    src/interpreter/Memory/ARC.cpp
    src/interpreter/Memory/MemorySafety.cpp
    src/interpreter/Memory/MemoryManager.cpp
    
    # Concurrency
    src/interpreter/Concurrency/ConcurrencyRuntime.cpp
    
    # IO and Network
    src/interpreter/IO/IORuntime.cpp
    src/interpreter/IONetworkIntegration.cpp
    src/interpreter/Network/NetworkRuntime.cpp
    
    # Semantic
    src/semantic/SymbolTable.cpp
    src/semantic/TypeSystem.cpp
    src/semantic/TypedAST.cpp
)

# Add CodeGen sources only if LLVM is available
if(LLVM_FOUND)
    list(APPEND CORE_SOURCES src/codegen/LLVMCodeGenerator.cpp)
endif()

# Create UI dynamic library
add_library(miniswift_ui SHARED 
    ${UI_SOURCES}
    src/interpreter/Value.cpp
    src/interpreter/Concurrency/ConcurrencyRuntime.cpp
)

# Set target properties for UI library
set_target_properties(miniswift_ui PROPERTIES
    OUTPUT_NAME "miniswift_ui"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Include directories for UI library
target_include_directories(miniswift_ui PRIVATE
    src
    third
)

# Link UI library with GTK4 if available
if(GTK4_FOUND)
    if(WIN32 AND TARGET gtk::gtk)
        target_link_libraries(miniswift_ui PRIVATE gtk::gtk)
    elseif(WIN32 AND TARGET gtk4::gtk4)
        target_link_libraries(miniswift_ui PRIVATE gtk4::gtk4)
    else()
        target_link_libraries(miniswift_ui PRIVATE ${GTK4_LIBRARIES})
    endif()
endif()

# Link UI library with system libraries
if(WIN32)
    target_link_libraries(miniswift_ui PRIVATE 
        kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32
        ws2_32 dbghelp psapi version
    )
else()
    target_link_libraries(miniswift_ui PRIVATE pthread dl m)
endif()

# Create shared library for core components
add_library(miniswift_core SHARED ${CORE_SOURCES})

# Set target properties for core library
set_target_properties(miniswift_core PROPERTIES
    OUTPUT_NAME "miniswift_core"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Include directories for core library
target_include_directories(miniswift_core PUBLIC
    src
    third
)

# Link libraries for core library
if(WIN32)
    target_link_libraries(miniswift_core PRIVATE
        kernel32 user32 ws2_32 dbghelp psapi version
    )
else()
    target_link_libraries(miniswift_core PRIVATE
        pthread
        dl
        m
    )
endif()

# Link LLVM libraries only if available
if(LLVM_FOUND)
    target_link_libraries(miniswift_core PRIVATE ${llvm_libs})
    target_include_directories(miniswift_core PRIVATE ${LLVM_INCLUDE_DIRS})
    target_compile_definitions(miniswift_core PRIVATE ${LLVM_DEFINITIONS_LIST} HAVE_LLVM)
endif()

# Add dependency on UI library and link it
add_dependencies(miniswift_core miniswift_ui)
target_link_libraries(miniswift_core PRIVATE miniswift_ui)

# Link core library with GTK4 if available
if(GTK4_FOUND)
    if(WIN32 AND TARGET gtk::gtk)
        target_link_libraries(miniswift_core PRIVATE gtk::gtk)
    elseif(WIN32 AND TARGET gtk4::gtk4)
        target_link_libraries(miniswift_core PRIVATE gtk4::gtk4)
    else()
        target_link_libraries(miniswift_core PRIVATE ${GTK4_LIBRARIES})
    endif()
    target_compile_definitions(miniswift_core PRIVATE HAVE_GTK4)
endif()



# =============================================================================
# Main Executable - Minimal sources, links to dynamic libraries
# =============================================================================

# Main executable sources - only entry point and essential files
set(MAIN_ONLY_SOURCES
    ${MAIN_SOURCES}
)

# Create the executable with minimal sources
add_executable(miniswift ${MAIN_ONLY_SOURCES})

# Add dependency on zstd
if(TARGET ${ZSTD_TARGET})
    add_dependencies(miniswift ${ZSTD_TARGET})
endif()

# Add dependencies on our dynamic libraries
add_dependencies(miniswift miniswift_core miniswift_ui)

# Link with dynamic libraries
target_link_libraries(miniswift PRIVATE 
    miniswift_core
    miniswift_ui
    miniswift_codegen_semantic
)

# Link LLVM libraries only if available
if(LLVM_FOUND)
    target_link_libraries(miniswift PRIVATE ${llvm_libs})
    target_compile_definitions(miniswift PRIVATE HAVE_LLVM)
endif()

# =============================================================================
# Test Executables
# =============================================================================

# Add C++ UI test executable
set(UI_TEST_SOURCES
    tests/SimpleUIAppTest.cpp
    src/ui/GTK4Backend.cpp
    src/ui/TextWidget.cpp
    src/ui/ButtonWidget.cpp
    src/ui/VStackWidget.cpp
    src/ui/HStackWidget.cpp
    src/ui/UIIntegration.cpp
    src/ui/UIRuntime.cpp
    src/interpreter/Value.cpp
    src/interpreter/Concurrency/ConcurrencyRuntime.cpp
)

add_executable(SimpleUIAppTest ${UI_TEST_SOURCES})
target_include_directories(SimpleUIAppTest PRIVATE src third)

# Create MinimalGTK4Test executable
set(MINIMAL_GTK4_TEST_SOURCES
    tests/MinimalGTK4Test.cpp
    src/ui/GTK4Backend.cpp
    src/ui/UIIntegration.cpp
    src/ui/TextWidget.cpp
    src/ui/ButtonWidget.cpp
    src/ui/VStackWidget.cpp
    src/ui/HStackWidget.cpp
    src/ui/UIRuntime.cpp
    src/interpreter/Value.cpp
    src/interpreter/Concurrency/ConcurrencyRuntime.cpp
)

add_executable(MinimalGTK4Test ${MINIMAL_GTK4_TEST_SOURCES})
target_include_directories(MinimalGTK4Test PRIVATE src third)

# Link test executables with GTK4 if available
if(GTK4_FOUND)
    if(WIN32 AND TARGET gtk::gtk)
        target_link_libraries(SimpleUIAppTest PRIVATE gtk::gtk)
        target_link_libraries(MinimalGTK4Test PRIVATE gtk::gtk)
    elseif(WIN32 AND TARGET gtk4::gtk4)
        target_link_libraries(SimpleUIAppTest PRIVATE gtk4::gtk4)
        target_link_libraries(MinimalGTK4Test PRIVATE gtk4::gtk4)
    else()
        target_link_libraries(SimpleUIAppTest PRIVATE ${GTK4_LIBRARIES})
        target_link_libraries(MinimalGTK4Test PRIVATE ${GTK4_LIBRARIES})
    endif()
endif()

# Link test executables with system libraries
if(WIN32)
    target_link_libraries(SimpleUIAppTest PRIVATE 
        kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32
        ws2_32 dbghelp psapi version
    )
    target_link_libraries(MinimalGTK4Test PRIVATE 
        kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32
        ws2_32 dbghelp psapi version
    )
else()
    target_link_libraries(SimpleUIAppTest PRIVATE pthread dl m)
    target_link_libraries(MinimalGTK4Test PRIVATE pthread dl m)
endif()

# Link with zstd target separately
if(TARGET ${ZSTD_TARGET})
    target_link_libraries(miniswift PRIVATE ${ZSTD_TARGET})
else()
    # Fallback to direct library linking
    target_link_libraries(miniswift PRIVATE ${ZSTD_LIBRARY})
endif()

# Link with GTK4 if available
if(GTK4_FOUND)
    message(STATUS "GTK4 found: ${GTK4_VERSION}")
    add_definitions(-DHAVE_GTK4)
    if(WIN32 AND TARGET gtk::gtk)
        # vcpkg 方式 - 使用 gtk::gtk 目标
        target_link_libraries(miniswift PRIVATE gtk::gtk)
        message(STATUS "Linking with vcpkg GTK target: gtk::gtk")
    elseif(WIN32 AND TARGET gtk4::gtk4)
        # 备用 vcpkg 方式
        target_link_libraries(miniswift PRIVATE gtk4::gtk4)
        message(STATUS "Linking with vcpkg GTK target: gtk4::gtk4")
    else()
        # pkg-config 方式
        include_directories(${GTK4_INCLUDE_DIRS})
        link_directories(${GTK4_LIBRARY_DIRS})
        add_compile_options(${GTK4_CFLAGS_OTHER})
        target_link_libraries(miniswift PRIVATE ${GTK4_LIBRARIES})
        message(STATUS "Linking with pkg-config GTK libraries")
    endif()
else()
    message(STATUS "GTK4 not found, UI will use mock implementation")
endif()
# Add platform-specific system libraries
if(APPLE)
    # macOS frameworks and libraries - no LLVM dependencies
    # Use standard system libraries
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
# =============================================================================
# CodeGen and Semantic Library (Shared) - Combined codegen and semantic analysis
# =============================================================================

# CodeGen and Semantic library sources
set(CODEGEN_SEMANTIC_SOURCES
    # Semantic
    src/semantic/SemanticAnalyzer.cpp
    src/semantic/SymbolTable.cpp
    src/semantic/TypeSystem.cpp
    src/semantic/TypedAST.cpp
)

# Add CodeGen sources only if LLVM is available
if(LLVM_FOUND)
    list(APPEND CODEGEN_SEMANTIC_SOURCES src/codegen/LLVMCodeGenerator.cpp)
endif()

# CodeGen and Semantic library headers
set(CODEGEN_SEMANTIC_HEADERS
    src/semantic/SemanticAnalyzer.h
    src/semantic/SymbolTable.h
    src/semantic/TypeSystem.h
    src/semantic/TypedAST.h
)

# Add CodeGen headers only if LLVM is available
if(LLVM_FOUND)
    list(APPEND CODEGEN_SEMANTIC_HEADERS src/codegen/LLVMCodeGenerator.h)
endif()

# Create shared library for codegen and semantic components
add_library(miniswift_codegen_semantic SHARED ${CODEGEN_SEMANTIC_SOURCES})

# Set target properties for codegen semantic library
set_target_properties(miniswift_codegen_semantic PROPERTIES
    OUTPUT_NAME "miniswift_codegen_semantic"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    PUBLIC_HEADER "${CODEGEN_SEMANTIC_HEADERS}"
)

# Include directories for codegen semantic library
target_include_directories(miniswift_codegen_semantic PRIVATE
    src
    third
)

# Add LLVM includes and definitions only if available
if(LLVM_FOUND)
    target_include_directories(miniswift_codegen_semantic PRIVATE ${LLVM_INCLUDE_DIRS})
    target_compile_definitions(miniswift_codegen_semantic PRIVATE ${LLVM_DEFINITIONS_LIST} HAVE_LLVM)
endif()

# Link libraries for codegen semantic library
if(WIN32)
    target_link_libraries(miniswift_codegen_semantic PRIVATE
        kernel32 user32 ws2_32 dbghelp psapi version
    )
else()
    target_link_libraries(miniswift_codegen_semantic PRIVATE
        pthread
        dl
        m
    )
endif()

# Link LLVM libraries only if available
if(LLVM_FOUND)
    target_link_libraries(miniswift_codegen_semantic PRIVATE ${llvm_libs})
endif()

# Install targets for codegen semantic library
install(TARGETS miniswift_codegen_semantic
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/miniswift
)

# Install additional codegen semantic headers
install(FILES ${CODEGEN_SEMANTIC_HEADERS}
    DESTINATION include/miniswift
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "UI library will be built as: miniswift_ui")
message(STATUS "Runtime library will be built as: miniswift_runtime")
message(STATUS "Core library will be built as: miniswift_core")
message(STATUS "CodeGen and Semantic library will be built as: miniswift_codegen_semantic")