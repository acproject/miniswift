cmake_minimum_required(VERSION 3.20)
project(MiniSwiftInterpreter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# 包含目录
include_directories(.)
include_directories(IO)
include_directories(Network)
include_directories(Memory)
include_directories(Concurrency)

# 查找线程库
find_package(Threads REQUIRED)

# IO模块源文件
set(IO_SOURCES
    IO/IORuntime.cpp
)

# Network模块源文件
set(NETWORK_SOURCES
    Network/NetworkRuntime.cpp
)

# 集成模块源文件
set(INTEGRATION_SOURCES
    IONetworkIntegration.cpp
    Value.cpp
    ErrorHandling.cpp
    OOP/Property.cpp
    OOP/Subscript.cpp
)

# 测试源文件
set(TEST_SOURCES
    IONetworkTest.cpp
)

# 创建IO库
add_library(miniswift_io STATIC ${IO_SOURCES})
target_link_libraries(miniswift_io Threads::Threads)

# 创建Network库
add_library(miniswift_network STATIC ${NETWORK_SOURCES})
target_link_libraries(miniswift_network Threads::Threads)

# 创建集成库
add_library(miniswift_integration STATIC ${INTEGRATION_SOURCES})
target_link_libraries(miniswift_integration miniswift_io miniswift_network Threads::Threads)

# 创建测试可执行文件
add_executable(io_network_test ${TEST_SOURCES})
target_link_libraries(io_network_test 
    miniswift_integration 
    miniswift_io 
    miniswift_network 
    Threads::Threads
)

# 创建平台兼容性测试
add_executable(platform_test PlatformTest.cpp)
target_link_libraries(platform_test miniswift_network)

# 设置输出目录
set_target_properties(io_network_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 安装规则
install(TARGETS miniswift_io miniswift_network miniswift_integration
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(TARGETS io_network_test
    RUNTIME DESTINATION bin
)

# 安装头文件
install(DIRECTORY IO/ DESTINATION include/miniswift/IO
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY Network/ DESTINATION include/miniswift/Network
    FILES_MATCHING PATTERN "*.h"
)

install(FILES IONetworkIntegration.h DESTINATION include/miniswift)