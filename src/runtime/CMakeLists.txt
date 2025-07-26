# MiniSwift Runtime Library CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

project(MiniSwiftRuntime
    VERSION 1.0.0
    DESCRIPTION "MiniSwift Runtime Library for compiled programs"
    LANGUAGES C CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /EHsc /GR)
    add_compile_options(/wd4244 /wd4267 /wd4996)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -frtti -pthread)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter
)

# Runtime library sources
set(RUNTIME_SOURCES
    # Memory management
    ../interpreter/Memory/ARC.cpp
    ../interpreter/Memory/MemorySafety.cpp
    ../interpreter/Memory/MemoryManager.cpp
    
    # Concurrency runtime
    ../interpreter/Concurrency/ConcurrencyRuntime.cpp
    
    # Runtime interface
    runtime_interface.cpp
)

# Runtime library headers
set(RUNTIME_HEADERS
    # Memory management
    ../interpreter/Memory/ARC.h
    ../interpreter/Memory/MemorySafety.h
    ../interpreter/Memory/MemoryManager.h
    
    # Concurrency runtime
    ../interpreter/Concurrency/ConcurrencyRuntime.h
    
    # Runtime interface
    runtime_interface.h
)

# Create static library
add_library(miniswift_runtime STATIC ${RUNTIME_SOURCES})

# Set target properties
set_target_properties(miniswift_runtime PROPERTIES
    OUTPUT_NAME "miniswift_runtime"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    PUBLIC_HEADER "${RUNTIME_HEADERS}"
)

# Platform-specific system libraries
if(WIN32)
    target_link_libraries(miniswift_runtime PRIVATE 
        kernel32 user32 ws2_32 dbghelp psapi version
    )
else()
    target_link_libraries(miniswift_runtime PRIVATE pthread dl m)
endif()

# Install targets
install(TARGETS miniswift_runtime
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/miniswift
)

# Install additional headers
install(FILES
    ../interpreter/Memory/ARC.h
    ../interpreter/Memory/MemorySafety.h
    ../interpreter/Memory/MemoryManager.h
    ../interpreter/Concurrency/ConcurrencyRuntime.h
    runtime_interface.h
    DESTINATION include/miniswift
)

# Print build information
message(STATUS "Building MiniSwift Runtime Library")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")