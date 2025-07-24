# Windows PowerShell 构建脚本 for MiniSwift
# 需要预先安装: Visual Studio 2019/2022, CMake, vcpkg, LLVM

param(
    [string]$BuildType = "Release",
    [switch]$Clean,
    [switch]$Help
)

if ($Help) {
    Write-Host "MiniSwift Windows PowerShell 构建脚本"
    Write-Host "用法: .\build_windows.ps1 [选项]"
    Write-Host ""
    Write-Host "选项:"
    Write-Host "  -BuildType <Release|Debug>  构建类型 (默认: Release)"
    Write-Host "  -Clean                     清理构建目录"
    Write-Host "  -Help                      显示此帮助信息"
    Write-Host ""
    Write-Host "示例:"
    Write-Host "  .\build_windows.ps1                    # Release 构建"
    Write-Host "  .\build_windows.ps1 -BuildType Debug   # Debug 构建"
    Write-Host "  .\build_windows.ps1 -Clean             # 清理构建目录"
    exit 0
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "MiniSwift Windows PowerShell 构建脚本" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# 设置变量
$BuildDir = "build_windows"
$ProjectRoot = Get-Location

# 清理构建目录
if ($Clean) {
    Write-Host "清理构建目录..." -ForegroundColor Yellow
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
        Write-Host "清理完成" -ForegroundColor Green
    } else {
        Write-Host "构建目录不存在，无需清理" -ForegroundColor Yellow
    }
    exit 0
}

Write-Host "构建类型: $BuildType" -ForegroundColor Green

# 检查必要工具
Write-Host "检查必要工具..." -ForegroundColor Yellow

# 检查 CMake
try {
    $cmakeVersion = cmake --version 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ CMake 已安装" -ForegroundColor Green
    } else {
        throw "CMake 未找到"
    }
} catch {
    Write-Host "✗ 错误: 未找到 CMake，请先安装 CMake" -ForegroundColor Red
    exit 1
}

# 检查 vcpkg
try {
    $vcpkgVersion = vcpkg version 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ vcpkg 已安装" -ForegroundColor Green
    } else {
        Write-Host "⚠ 警告: 未找到 vcpkg，请确保已安装 vcpkg 并添加到 PATH" -ForegroundColor Yellow
    }
} catch {
    Write-Host "⚠ 警告: 未找到 vcpkg" -ForegroundColor Yellow
}

# 检查 Visual Studio
$vsInstallPath = @(
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat"
) | Where-Object { Test-Path $_ } | Select-Object -First 1

if ($vsInstallPath) {
    Write-Host "✓ Visual Studio 已安装: $vsInstallPath" -ForegroundColor Green
} else {
    Write-Host "⚠ 警告: 未找到 Visual Studio 2019/2022" -ForegroundColor Yellow
}

# 创建构建目录
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}
Set-Location $BuildDir

# 检查并设置 vcpkg 工具链
$vcpkgToolchain = $env:CMAKE_TOOLCHAIN_FILE
if (-not $vcpkgToolchain) {
    Write-Host "尝试自动检测 vcpkg..." -ForegroundColor Yellow
    $possiblePaths = @(
        "C:\vcpkg\scripts\buildsystems\vcpkg.cmake",
        "C:\tools\vcpkg\scripts\buildsystems\vcpkg.cmake",
        "${env:VCPKG_ROOT}\scripts\buildsystems\vcpkg.cmake"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            $vcpkgToolchain = $path
            Write-Host "✓ 找到 vcpkg: $vcpkgToolchain" -ForegroundColor Green
            break
        }
    }
    
    if (-not $vcpkgToolchain) {
        Write-Host "⚠ 警告: 未找到 vcpkg 工具链文件" -ForegroundColor Yellow
        Write-Host "请手动设置 CMAKE_TOOLCHAIN_FILE 环境变量" -ForegroundColor Yellow
        Write-Host "例如: `$env:CMAKE_TOOLCHAIN_FILE = 'C:\vcpkg\scripts\buildsystems\vcpkg.cmake'" -ForegroundColor Yellow
    }
}

# 安装依赖 (如果使用 vcpkg)
if ($vcpkgToolchain -and (Get-Command vcpkg -ErrorAction SilentlyContinue)) {
    Write-Host "安装 zstd 依赖..." -ForegroundColor Yellow
    try {
        vcpkg install zstd:x64-windows-static
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ zstd 安装成功" -ForegroundColor Green
        } else {
            Write-Host "⚠ 警告: zstd 安装失败，将尝试继续构建" -ForegroundColor Yellow
        }
    } catch {
        Write-Host "⚠ 警告: zstd 安装失败，将尝试继续构建" -ForegroundColor Yellow
    }
}

# 配置项目
Write-Host "配置项目..." -ForegroundColor Yellow
$cmakeArgs = @(
    "..",
    "-G", "Visual Studio 17 2022",
    "-A", "x64",
    "-DCMAKE_BUILD_TYPE=$BuildType"
)

if ($vcpkgToolchain) {
    Write-Host "使用 vcpkg: $vcpkgToolchain" -ForegroundColor Yellow
    $cmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$vcpkgToolchain"
    $cmakeArgs += "-DVCPKG_TARGET_TRIPLET=x64-windows-static"
} else {
    Write-Host "⚠ 警告: 未找到 vcpkg，将尝试不使用 vcpkg 构建" -ForegroundColor Yellow
    Write-Host "如需使用 vcpkg，请安装并设置 VCPKG_ROOT 环境变量" -ForegroundColor Yellow
}

try {
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) {
        throw "CMake 配置失败"
    }
    Write-Host "✓ 项目配置成功" -ForegroundColor Green
} catch {
    Write-Host "✗ 错误: CMake 配置失败" -ForegroundColor Red
    Set-Location $ProjectRoot
    exit 1
}

# 构建项目
Write-Host "构建项目..." -ForegroundColor Yellow
try {
    cmake --build . --config $BuildType --parallel
    if ($LASTEXITCODE -ne 0) {
        throw "构建失败"
    }
    Write-Host "✓ 项目构建成功" -ForegroundColor Green
} catch {
    Write-Host "✗ 错误: 构建失败" -ForegroundColor Red
    Set-Location $ProjectRoot
    exit 1
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "构建成功!" -ForegroundColor Green
Write-Host "可执行文件位置: $BuildDir\$BuildType\miniswift.exe" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan

# 测试运行
Write-Host "测试运行..." -ForegroundColor Yellow
$exePath = "$BuildType\miniswift.exe"
if (Test-Path $exePath) {
    try {
        & ".\$exePath" --help
        Write-Host "✓ 可执行文件运行正常" -ForegroundColor Green
    } catch {
        Write-Host "⚠ 警告: 可执行文件运行测试失败" -ForegroundColor Yellow
    }
} elseif (Test-Path "miniswift.exe") {
    try {
        .\miniswift.exe --help
        Write-Host "✓ 可执行文件运行正常" -ForegroundColor Green
    } catch {
        Write-Host "⚠ 警告: 可执行文件运行测试失败" -ForegroundColor Yellow
    }
} else {
    Write-Host "⚠ 警告: 未找到可执行文件" -ForegroundColor Yellow
}

Set-Location $ProjectRoot
Write-Host "构建完成!" -ForegroundColor Green