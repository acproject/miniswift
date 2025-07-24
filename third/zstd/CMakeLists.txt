# zstd库的跨平台构建脚本
cmake_minimum_required(VERSION 3.16)
project(zstd_build)

# 检测操作系统
if(WIN32)
    # Windows平台使用vcpkg管理zstd
    message(STATUS "Windows平台检测到，尝试使用vcpkg管理zstd依赖")
    
    # 查找vcpkg安装的zstd
    find_package(zstd CONFIG)
    
    if(zstd_FOUND)
        message(STATUS "找到vcpkg安装的zstd")
        # 创建导入目标
        add_library(zstd_target INTERFACE)
        target_link_libraries(zstd_target INTERFACE zstd::libzstd_static)
        
        # 设置变量供父项目使用
        set(ZSTD_LIBRARY zstd::libzstd_static PARENT_SCOPE)
        set(ZSTD_INCLUDE_DIR "" PARENT_SCOPE)  # vcpkg会自动处理包含路径
    else()
        message(WARNING "未找到vcpkg安装的zstd，请确保已通过vcpkg安装zstd包")
        message(STATUS "请运行: vcpkg install zstd:x64-windows")
        message(FATAL_ERROR "zstd依赖未找到，无法继续构建")
    endif()
    
else()
    # 非Windows平台使用源码编译
    message(STATUS "非Windows平台检测到，使用源码编译zstd")
    
    include(ExternalProject)
    
    # 获取目标架构
    if(CMAKE_OSX_ARCHITECTURES)
        set(TARGET_ARCH ${CMAKE_OSX_ARCHITECTURES})
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(TARGET_ARCH "arm64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(TARGET_ARCH "x86_64")
    else()
        set(TARGET_ARCH ${CMAKE_SYSTEM_PROCESSOR})
    endif()
    
    # 设置安装目录
    set(ZSTD_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/install")
    
    # 下载并编译zstd
    ExternalProject_Add(
        zstd_external
        URL https://github.com/facebook/zstd/releases/download/v1.5.6/zstd-1.5.6.tar.gz
        URL_HASH SHA256=8c29e06cf42aacc1eafc4077ae2ec6c6fcb96a626157e0593d5e82a34fd403c1
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
        SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/src/zstd"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND make -C <SOURCE_DIR>/lib libzstd.a CFLAGS=-arch\ ${TARGET_ARCH}
        INSTALL_COMMAND 
            ${CMAKE_COMMAND} -E make_directory "${ZSTD_INSTALL_DIR}/lib" &&
            ${CMAKE_COMMAND} -E make_directory "${ZSTD_INSTALL_DIR}/include" &&
            ${CMAKE_COMMAND} -E copy "<SOURCE_DIR>/lib/libzstd.a" "${ZSTD_INSTALL_DIR}/lib/" &&
            ${CMAKE_COMMAND} -E copy "<SOURCE_DIR>/lib/zstd.h" "${ZSTD_INSTALL_DIR}/include/"
        BUILD_IN_SOURCE 1
    )
    
    # 创建导入目标
    add_library(zstd_target STATIC IMPORTED GLOBAL)
    set_target_properties(zstd_target PROPERTIES
        IMPORTED_LOCATION "${ZSTD_INSTALL_DIR}/lib/libzstd.a"
        INTERFACE_INCLUDE_DIRECTORIES "${ZSTD_INSTALL_DIR}/include"
    )
    add_dependencies(zstd_target zstd_external)
    
    # 设置变量供父项目使用
    set(ZSTD_LIBRARY "${ZSTD_INSTALL_DIR}/lib/libzstd.a" PARENT_SCOPE)
    set(ZSTD_INCLUDE_DIR "${ZSTD_INSTALL_DIR}/include" PARENT_SCOPE)
    
endif()

# 导出目标供父项目使用
set(ZSTD_TARGET zstd_target PARENT_SCOPE)
message(STATUS "zstd构建配置完成")