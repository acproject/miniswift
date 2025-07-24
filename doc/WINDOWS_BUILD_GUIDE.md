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

### 4. 安装 vcpkg（包管理器）

**vcpkg** 是 Microsoft 推荐的 C++ 包管理器，用于管理第三方库依赖。

```cmd
# 克隆 vcpkg 到推荐位置
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg

# 运行引导脚本
.\bootstrap-vcpkg.bat

# 集成到 Visual Studio（重要！）
.\vcpkg integrate install
```

**设置环境变量：**
1. 打开系统环境变量设置
2. 添加新的环境变量：
   - 变量名：`VCPKG_ROOT`
   - 变量值：`C:\vcpkg`
3. 将 `C:\vcpkg` 添加到 PATH 环境变量

**验证安装：**
```cmd
vcpkg version
vcpkg integrate list
```

### 5. 安装 zstd 库

使用 vcpkg 安装 zstd 压缩库：

```cmd
# 安装 x64 版本（推荐）
vcpkg install zstd:x64-windows

# 验证安装
vcpkg list zstd
```

## 构建项目

### 方法 1: 使用构建脚本 (推荐)

项目提供了自动化构建脚本，会自动检测和配置 vcpkg：

**使用批处理脚本：**
```cmd
git clone <repository-url>
cd miniswift
.\scripts\build_windows.bat
```

**构建 Debug 版本：**
```cmd
.\scripts\build_windows.bat debug
```

**使用 PowerShell 脚本：**
```powershell
git clone <repository-url>
cd miniswift
.\scripts\build_windows.ps1
```

### 方法 2: 手动构建

如果需要手动控制构建过程：

1. 克隆项目：
   ```cmd
   git clone <repository-url>
   cd miniswift
   ```

2. 创建构建目录：
   ```cmd
   mkdir build_windows
   cd build_windows
   ```

3. 配置项目：
   
   **如果已正确设置 VCPKG_ROOT 环境变量：**
   ```cmd
   cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows
   ```
   
   **如果 vcpkg 安装在默认位置：**
   ```cmd
   cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows
   ```
   
   **不使用 vcpkg（不推荐）：**
   ```cmd
   cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
   ```

4. 构建项目：
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

**错误信息：**
```
clang++: error: no such file or directory: '/DLLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1'
```

**可能原因：**
编译器标志格式不正确。在 Windows 平台上：
- MSVC 编译器使用 `/D` 前缀定义宏
- Clang 编译器使用 `-D` 前缀定义宏
- CMake 可能混合了不同编译器的标志格式

**解决方案：**

1. **确保使用正确的编译器：**
   - **推荐使用 MSVC 编译器**（Visual Studio 2022）
   - 避免混合使用不同的编译器工具链
   - 确保 CMake 检测到正确的编译器

2. **检查编译器检测：**
   ```cmd
   # 在 CMake 配置时查看输出
   cmake .. -G "Visual Studio 17 2022" -A x64
   # 应该显示：
   # -- The CXX compiler identification is MSVC
   ```

3. **清理构建目录：**
   ```cmd
   rmdir /s build
   mkdir build
   cd build
   ```

4. **强制使用 MSVC 编译器：**
   ```cmd
   cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_CXX_COMPILER="cl.exe"
   ```

5. **如果必须使用 Clang：**
   确保项目配置能正确处理 Clang 标志格式（项目已支持自动检测）

### 3. zstd 库链接错误

如果遇到类似 `Target "miniswift" links to: zstd::libzstd_static but the target was not found` 的错误，这通常是因为：

**可能原因**:
1. vcpkg 没有正确安装 zstd 库
2. CMake 没有找到 vcpkg 工具链
3. zstd 库的目标名称不匹配（不同版本的 vcpkg 可能使用不同的目标名称）
4. vcpkg 未正确集成到 Visual Studio

**解决方案**:
1. **确保 vcpkg 已正确安装和集成：**
   ```cmd
   # 克隆 vcpkg（如果尚未安装）
   git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg
   .\bootstrap-vcpkg.bat
   
   # 集成到 Visual Studio
   .\vcpkg integrate install
   ```

2. **安装 zstd：**
   ```cmd
   # 安装 x64 版本（推荐）
   vcpkg install zstd:x64-windows
   
   # 验证安装
   vcpkg list zstd
   ```

3. **检查 zstd 目标：**
   项目现在会自动检测可用的 zstd 目标名称：
   - `zstd::libzstd_static`（静态库）
   - `zstd::libzstd_shared`（动态库）
   - `zstd::zstd`（通用目标）

4. **确保 CMake 使用了正确的 vcpkg 工具链：**
   ```cmd
   cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake"
   ```

5. **检查 vcpkg 集成状态：**
   ```cmd
   vcpkg integrate list
   vcpkg list zstd
   ```

6. **如果仍然失败，尝试重新安装：**
   ```cmd
   vcpkg remove zstd:x64-windows
   vcpkg install zstd:x64-windows
   ```

### 4. vcpkg 依赖安装失败

确保 vcpkg 已正确安装并集成到 Visual Studio。检查网络连接，某些依赖可能需要从网络下载。

### 5. 链接错误

确保使用了正确的 vcpkg triplet (`x64-windows-static`) 并且所有依赖都使用相同的运行时库。

### 6. 编译器版本不兼容

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