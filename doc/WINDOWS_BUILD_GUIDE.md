# Windows 构建指南

本指南将帮助您在 Windows 环境下构建 MiniSwift 项目。

## 系统要求

- Windows 10 或更高版本
- Visual Studio 2019 或 2022 (Community 版本即可)
- CMake 3.16 或更高版本
- LLVM 12.0 或更高版本
- vcpkg (推荐用于依赖管理)

## 安装步骤

### 1. 安装 Visual Studio

1. 下载并安装 [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/)
2. 在安装时选择以下工作负载：
   - "使用 C++ 的桌面开发"
   - "CMake tools for Visual Studio"

### 2. 安装 CMake

1. 下载并安装 [CMake](https://cmake.org/download/)
2. 确保在安装时选择 "Add CMake to the system PATH"

### 3. 安装 LLVM

#### 选项 A: 使用预编译二进制文件 (推荐)

1. 从 [LLVM Releases](https://github.com/llvm/llvm-project/releases) 下载适合您系统的预编译版本
2. 安装到默认位置 (通常是 `C:\Program Files\LLVM`)
3. 将 LLVM 的 bin 目录添加到系统 PATH

#### 选项 B: 使用 Chocolatey

```powershell
choco install llvm
```

### 4. 安装 vcpkg (推荐)

1. 克隆 vcpkg 仓库：
   ```cmd
   git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg
   .\bootstrap-vcpkg.bat
   ```

2. 集成到 Visual Studio：
   ```cmd
   .\vcpkg integrate install
   ```

3. 设置环境变量：
   ```cmd
   set CMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
   ```

### 5. 安装依赖

使用 vcpkg 安装 zstd：
```cmd
vcpkg install zstd:x64-windows-static
```

## 构建项目

### 方法 1: 使用构建脚本 (推荐)

1. 打开命令提示符或 PowerShell
2. 导航到项目根目录
3. 运行构建脚本：
   ```cmd
   scripts\build_windows.bat
   ```

   或者构建 Debug 版本：
   ```cmd
   scripts\build_windows.bat debug
   ```

### 方法 2: 手动构建

1. 创建构建目录：
   ```cmd
   mkdir build_windows
   cd build_windows
   ```

2. 配置项目：
   ```cmd
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
   ```

3. 构建项目：
   ```cmd
   cmake --build . --config Release --parallel
   ```

### 方法 3: 使用 Visual Studio

1. 打开 Visual Studio
2. 选择 "打开本地文件夹"
3. 选择 MiniSwift 项目根目录
4. Visual Studio 会自动检测 CMakeLists.txt 并配置项目
5. 在解决方案资源管理器中右键点击 CMakeLists.txt，选择 "生成"

## 运行测试

构建完成后，可执行文件将位于：
- 使用脚本构建: `build_windows\Release\miniswift.exe`
- 使用 Visual Studio: `out\build\x64-Release\miniswift.exe`

测试运行：
```cmd
miniswift.exe --help
echo print("Hello, World!") | miniswift.exe
```

## 常见问题

### 1. CMake 找不到 LLVM

确保 LLVM 已正确安装并添加到 PATH。您也可以手动指定 LLVM 路径：
```cmd
cmake .. -DLLVM_DIR="C:\Program Files\LLVM\lib\cmake\llvm"
```

### 2. 编译器标志错误

如果遇到类似 `clang++: error: no such file or directory: '/DLLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1'` 的错误，这是因为编译器标志格式不正确。

**解决方案**: 确保 CMakeLists.txt 中使用 `-D` 前缀而不是 `/D`：
```cmake
# 正确的方式
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1")

# 错误的方式
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DLLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1")
```

### 3. vcpkg 依赖安装失败

确保 vcpkg 已正确安装并集成到 Visual Studio。检查网络连接，某些依赖可能需要从网络下载。

### 4. 链接错误

确保使用了正确的 vcpkg triplet (`x64-windows-static`) 并且所有依赖都使用相同的运行时库。

### 5. 编译器版本不兼容

确保 Visual Studio、LLVM 和 vcpkg 使用的编译器版本兼容。推荐使用 Visual Studio 2019 或 2022。

## 开发环境设置

### Visual Studio Code

如果您更喜欢使用 VS Code：

1. 安装 C/C++ 扩展
2. 安装 CMake Tools 扩展
3. 打开项目文件夹
4. 按 Ctrl+Shift+P，选择 "CMake: Configure"
5. 选择适当的工具包 (Visual Studio 编译器)

### 环境变量

为了方便开发，建议设置以下环境变量：

```cmd
set CMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
set VCPKG_DEFAULT_TRIPLET=x64-windows-static
set LLVM_DIR=C:\Program Files\LLVM\lib\cmake\llvm
```

## 性能优化

- 使用 Release 构建获得最佳性能
- 启用并行构建: `cmake --build . --parallel`
- 考虑使用 Ninja 生成器以获得更快的构建速度：
  ```cmd
  cmake .. -G Ninja
  ninja
  ```

## 故障排除

如果遇到问题，请检查：

1. 所有必需的工具都已正确安装
2. 环境变量设置正确
3. 防火墙或杀毒软件没有阻止构建过程
4. 磁盘空间充足 (构建过程可能需要几 GB 空间)

如果问题仍然存在，请查看构建日志或在项目仓库中提交 issue。