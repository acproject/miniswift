@echo off
REM Windows构建脚本 for MiniSwift
REM 需要预先安装: Visual Studio 2019/2022, CMake, vcpkg, LLVM

echo ========================================
echo MiniSwift Windows 构建脚本
echo ========================================

REM 检查必要工具
echo 检查必要工具...

where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo 错误: 未找到 CMake，请先安装 CMake
    pause
    exit /b 1
)

where vcpkg >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo 警告: 未找到 vcpkg，请确保已安装 vcpkg 并添加到 PATH
    echo 或者手动设置 CMAKE_TOOLCHAIN_FILE
)

REM 设置变量
set BUILD_DIR=build_windows
set CMAKE_BUILD_TYPE=Release

REM 解析命令行参数
if "%1"=="debug" set CMAKE_BUILD_TYPE=Debug
if "%1"=="clean" (
    echo 清理构建目录...
    if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
    echo 清理完成
    exit /b 0
)

echo 构建类型: %CMAKE_BUILD_TYPE%

REM 创建构建目录
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

REM 检查是否设置了 vcpkg 工具链
if not defined CMAKE_TOOLCHAIN_FILE (
    echo 尝试自动检测 vcpkg...
    if exist "C:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
        set CMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
        echo 找到 vcpkg: %CMAKE_TOOLCHAIN_FILE%
    ) else if exist "C:\tools\vcpkg\scripts\buildsystems\vcpkg.cmake" (
        set CMAKE_TOOLCHAIN_FILE=C:\tools\vcpkg\scripts\buildsystems\vcpkg.cmake
        echo 找到 vcpkg: %CMAKE_TOOLCHAIN_FILE%
    ) else (
        echo 警告: 未找到 vcpkg 工具链文件
        echo 请手动设置 CMAKE_TOOLCHAIN_FILE 环境变量
        echo 例如: set CMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
    )
)

REM 安装依赖 (如果使用 vcpkg)
if defined CMAKE_TOOLCHAIN_FILE (
    echo 安装 zstd 依赖...
    vcpkg install zstd:x64-windows-static
    if %ERRORLEVEL% neq 0 (
        echo 警告: zstd 安装失败，将尝试继续构建
    )
)

REM 配置项目
echo 配置项目...
if defined CMAKE_TOOLCHAIN_FILE (
    cmake .. -DCMAKE_BUILD_TYPE=%CMAKE_BUILD_TYPE% -DCMAKE_TOOLCHAIN_FILE="%CMAKE_TOOLCHAIN_FILE%" -DVCPKG_TARGET_TRIPLET=x64-windows-static
) else (
    cmake .. -DCMAKE_BUILD_TYPE=%CMAKE_BUILD_TYPE%
)

if %ERRORLEVEL% neq 0 (
    echo 错误: CMake 配置失败
    cd ..
    pause
    exit /b 1
)

REM 构建项目
echo 构建项目...
cmake --build . --config %CMAKE_BUILD_TYPE% --parallel

if %ERRORLEVEL% neq 0 (
    echo 错误: 构建失败
    cd ..
    pause
    exit /b 1
)

echo ========================================
echo 构建成功!
echo 可执行文件位置: %BUILD_DIR%\%CMAKE_BUILD_TYPE%\miniswift.exe
echo ========================================

REM 测试运行
echo 测试运行...
if exist "%CMAKE_BUILD_TYPE%\miniswift.exe" (
    "%CMAKE_BUILD_TYPE%\miniswift.exe" --help
) else if exist "miniswift.exe" (
    miniswift.exe --help
) else (
    echo 警告: 未找到可执行文件
)

cd ..
pause