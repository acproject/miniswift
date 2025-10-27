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
    src/ui/TGUIBackend.cpp
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


# Find TGUI (and its SFML backend) for the UI
find_package(TGUI QUIET)
if(TGUI_FOUND)
    message(STATUS "TGUI found: ${TGUI_VERSION}")
    message(STATUS "TGUI include dirs: ${TGUI_INCLUDE_DIRS}")
    message(STATUS "TGUI libraries: ${TGUI_LIBRARIES}")
else()
    message(STATUS "TGUI not found; TGUI backend will be disabled")
endif()

# Find SFML (required by TGUI)
find_package(SFML QUIET COMPONENTS Graphics Window System)
if(SFML_FOUND)
    message(STATUS "SFML found: ${SFML_VERSION}")
else()
    message(STATUS "SFML not found; required for TGUI backend")
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

# GTK4 support has been removed in favor of TGUI backend
# All GTK4-related functionality has been replaced with TGUI



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

# Link UI library with TGUI if available
if(TGUI_FOUND AND SFML_FOUND)
    message(STATUS "Linking UI library with TGUI and SFML")
    
    # Link with TGUI
    if(TARGET TGUI::TGUI)
        target_link_libraries(miniswift_ui PRIVATE TGUI::TGUI)
        message(STATUS "Linked with TGUI::TGUI target")
    elseif(TARGET tgui)
        target_link_libraries(miniswift_ui PRIVATE tgui)
        message(STATUS "Linked with tgui target")
    else()
        target_link_libraries(miniswift_ui PRIVATE ${TGUI_LIBRARIES})
        target_include_directories(miniswift_ui PRIVATE ${TGUI_INCLUDE_DIRS})
        message(STATUS "Linked with TGUI libraries: ${TGUI_LIBRARIES}")
    endif()
    
    # Link with SFML
    if(TARGET sfml-graphics AND TARGET sfml-window AND TARGET sfml-system)
        target_link_libraries(miniswift_ui PRIVATE sfml-graphics sfml-window sfml-system)
        message(STATUS "Linked with SFML targets")
    else()
        target_link_libraries(miniswift_ui PRIVATE ${SFML_LIBRARIES})
        target_include_directories(miniswift_ui PRIVATE ${SFML_INCLUDE_DIRS})
        message(STATUS "Linked with SFML libraries: ${SFML_LIBRARIES}")
    endif()
    
    target_compile_definitions(miniswift_ui PRIVATE HAVE_TGUI)
else()
    message(STATUS "TGUI or SFML not found; UI will use mock implementation")
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

# Link core library with TGUI if available (for transitive UI usage)
if(TGUI_FOUND AND SFML_FOUND)
    message(STATUS "Linking core library with TGUI and SFML")
    
    # Link with TGUI
    if(TARGET TGUI::TGUI)
        target_link_libraries(miniswift_core PRIVATE TGUI::TGUI)
        message(STATUS "Core linked with TGUI::TGUI target")
    elseif(TARGET tgui)
        target_link_libraries(miniswift_core PRIVATE tgui)
        message(STATUS "Core linked with tgui target")
    else()
        target_link_libraries(miniswift_core PRIVATE ${TGUI_LIBRARIES})
        target_include_directories(miniswift_core PRIVATE ${TGUI_INCLUDE_DIRS})
        message(STATUS "Core linked with TGUI libraries: ${TGUI_LIBRARIES}")
    endif()
    
    # Link with SFML
    if(TARGET sfml-graphics AND TARGET sfml-window AND TARGET sfml-system)
        target_link_libraries(miniswift_core PRIVATE sfml-graphics sfml-window sfml-system)
        message(STATUS "Core linked with SFML targets")
    else()
        target_link_libraries(miniswift_core PRIVATE ${SFML_LIBRARIES})
        target_include_directories(miniswift_core PRIVATE ${SFML_INCLUDE_DIRS})
        message(STATUS "Core linked with SFML libraries: ${SFML_LIBRARIES}")
    endif()
    
    target_compile_definitions(miniswift_core PRIVATE HAVE_TGUI)
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

# TGUI-based tests are built when TGUI is available
if(TGUI_FOUND AND SFML_FOUND)
    # Add C++ UI test executable (TGUI-based)
    set(UI_TEST_SOURCES
        tests/SimpleUIAppTest.cpp
        src/ui/TGUIBackend.cpp
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

    # Create MinimalTGUITest executable (TGUI-only)
    set(MINIMAL_TGUI_TEST_SOURCES
        tests/MinimalTGUITest.cpp
        src/ui/TGUIBackend.cpp
        src/ui/UIIntegration.cpp
        src/ui/TextWidget.cpp
        src/ui/ButtonWidget.cpp
        src/ui/VStackWidget.cpp
        src/ui/HStackWidget.cpp
        src/ui/UIRuntime.cpp
        src/interpreter/Value.cpp
        src/interpreter/Concurrency/ConcurrencyRuntime.cpp
    )

    add_executable(MinimalTGUITest ${MINIMAL_TGUI_TEST_SOURCES})
    target_include_directories(MinimalTGUITest PRIVATE src third)
else()
    message(STATUS "TGUI or SFML not found; skipping TGUI-based UI tests (SimpleUIAppTest, MinimalTGUITest)")
endif()

# Link test executables with TGUI and SFML if available
if(TGUI_FOUND AND SFML_FOUND)
    target_link_libraries(SimpleUIAppTest PRIVATE tgui sfml-graphics sfml-window sfml-system)
    target_link_libraries(MinimalTGUITest PRIVATE tgui sfml-graphics sfml-window sfml-system)
    target_compile_definitions(SimpleUIAppTest PRIVATE HAVE_TGUI)
    target_compile_definitions(MinimalTGUITest PRIVATE HAVE_TGUI)
endif()

# Link test executables with system libraries
if(TGUI_FOUND AND SFML_FOUND)
    if(WIN32)
        target_link_libraries(SimpleUIAppTest PRIVATE 
            kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32
            ws2_32 dbghelp psapi version
        )
        target_link_libraries(MinimalTGUITest PRIVATE 
            kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32
            ws2_32 dbghelp psapi version
        )
    else()
        target_link_libraries(SimpleUIAppTest PRIVATE pthread dl m)
        target_link_libraries(MinimalTGUITest PRIVATE pthread dl m)
    endif()
endif()

# Link with zstd target separately
if(TARGET ${ZSTD_TARGET})
    target_link_libraries(miniswift PRIVATE ${ZSTD_TARGET})
else()
    # Fallback to direct library linking
    target_link_libraries(miniswift PRIVATE ${ZSTD_LIBRARY})
endif()

# Link with TGUI if available
if(TGUI_FOUND AND SFML_FOUND)
    message(STATUS "Linking main executable with TGUI and SFML")
    
    # Link with TGUI
    if(TARGET TGUI::TGUI)
        target_link_libraries(miniswift PRIVATE TGUI::TGUI)
        message(STATUS "Main executable linked with TGUI::TGUI target")
    elseif(TARGET tgui)
        target_link_libraries(miniswift PRIVATE tgui)
        message(STATUS "Main executable linked with tgui target")
    else()
        target_link_libraries(miniswift PRIVATE ${TGUI_LIBRARIES})
        target_include_directories(miniswift PRIVATE ${TGUI_INCLUDE_DIRS})
        message(STATUS "Main executable linked with TGUI libraries: ${TGUI_LIBRARIES}")
    endif()
    
    # Link with SFML
    if(TARGET sfml-graphics AND TARGET sfml-window AND TARGET sfml-system)
        target_link_libraries(miniswift PRIVATE sfml-graphics sfml-window sfml-system)
        message(STATUS "Main executable linked with SFML targets")
    else()
        target_link_libraries(miniswift PRIVATE ${SFML_LIBRARIES})
        target_include_directories(miniswift PRIVATE ${SFML_INCLUDE_DIRS})
        message(STATUS "Main executable linked with SFML libraries: ${SFML_LIBRARIES}")
    endif()
    
    target_compile_definitions(miniswift PRIVATE HAVE_TGUI)
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