#!/bin/bash

# 直接使用LLVM 20工具链编译MiniSwift项目的脚本
# 这个脚本绕过CMake，直接调用LLVM 20的编译器和链接器

set -e  # 遇到错误立即退出

# 检查LLVM20_HOME环境变量
if [ -z "$LLVM20_HOME" ]; then
    echo "错误: LLVM20_HOME环境变量未设置"
    echo "请设置LLVM20_HOME指向您的LLVM 20安装目录"
    exit 1
fi

echo "使用LLVM 20工具链: $LLVM20_HOME"

# 设置编译器和工具路径
CLANG="$LLVM20_HOME/bin/clang++"
LLVM_CONFIG="$LLVM20_HOME/bin/llvm-config"

# 检查工具是否存在
if [ ! -f "$CLANG" ]; then
    echo "错误: 找不到clang++: $CLANG"
    exit 1
fi

if [ ! -f "$LLVM_CONFIG" ]; then
    echo "错误: 找不到llvm-config: $LLVM_CONFIG"
    exit 1
fi

# 获取LLVM配置
LLVM_CXXFLAGS_RAW=$("$LLVM_CONFIG" --cxxflags)
LLVM_LDFLAGS_RAW=$("$LLVM_CONFIG" --ldflags)
LLVM_LIBS=$("$LLVM_CONFIG" --libs core support irreader executionengine mcjit native)
LLVM_SYSTEM_LIBS=$("$LLVM_CONFIG" --system-libs)

# 过滤LLVM标志，移除禁用异常和RTTI的标志
LLVM_CXXFLAGS=$(echo "$LLVM_CXXFLAGS_RAW" | sed 's/-fno-exceptions//g' | sed 's/-fno-rtti//g')

# 过滤链接标志，移除有问题的库路径
LLVM_LDFLAGS=$(echo "$LLVM_LDFLAGS_RAW" | sed 's|/opt/homebrew/lib/libzstd.a||g' | sed 's/-L\/opt\/homebrew\/lib//g')

# 过滤系统库，移除有问题的libzstd.a路径，并添加我们自己编译的zstd库
LLVM_SYSTEM_LIBS=$(echo "$LLVM_SYSTEM_LIBS" | sed 's|/opt/homebrew/lib/libzstd.a||g')
LLVM_SYSTEM_LIBS="$LLVM_SYSTEM_LIBS $(pwd)/libzstd.a"

echo "LLVM C++标志: $LLVM_CXXFLAGS"
echo "LLVM 链接标志: $LLVM_LDFLAGS"
echo "LLVM 库: $LLVM_LIBS"

# 创建构建目录
BUILD_DIR="build_llvm20"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 源文件列表
SOURCE_FILES=(
    "../src/main.cpp"
    "../src/lexer/Lexer.cpp"
    "../src/parser/Parser.cpp"
    "../src/parser/AstPrinter.cpp"
    "../src/parser/AccessControl.cpp"
    "../src/parser/ErrorNodes.cpp"
    "../src/interpreter/Interpreter.cpp"
    "../src/interpreter/Environment.cpp"
    "../src/interpreter/Value.cpp"
    "../src/interpreter/OOP/Property.cpp"
    "../src/interpreter/OOP/Method.cpp"
    "../src/interpreter/OOP/MethodInterpreter.cpp"
    "../src/interpreter/OOP/Constructor.cpp"
    "../src/interpreter/OOP/Inheritance.cpp"
    "../src/interpreter/OOP/Subscript.cpp"
    "../src/interpreter/OOP/Optional.cpp"
    "../src/interpreter/Memory/ARC.cpp"
    "../src/interpreter/Memory/MemorySafety.cpp"
    "../src/interpreter/Memory/MemoryManager.cpp"
    "../src/semantic/SemanticAnalyzer.cpp"
    "../src/semantic/SymbolTable.cpp"
    "../src/semantic/TypeSystem.cpp"
    "../src/semantic/TypedAST.cpp"
    "../src/codegen/LLVMCodeGenerator.cpp"
)

# 编译标志 - 添加系统头文件路径以解决兼容性问题，启用异常和RTTI，禁用zstd
CXXFLAGS="-std=c++20 -frtti -fexceptions -I../src -arch arm64 -isysroot $(xcrun --show-sdk-path) -I$(xcrun --show-sdk-path)/usr/include -DLLVM_ENABLE_ZSTD=0"

echo "开始编译..."

# 编译每个源文件为目标文件
OBJECT_FILES=()
for source in "${SOURCE_FILES[@]}"; do
    if [ -f "$source" ]; then
        object_file="$(basename "$source" .cpp).o"
        echo "编译: $source -> $object_file"
        "$CLANG" $CXXFLAGS $LLVM_CXXFLAGS -c "$source" -o "$object_file"
        OBJECT_FILES+=("$object_file")
    else
        echo "警告: 源文件不存在: $source"
    fi
done

echo "开始链接..."

# 链接标志 - 直接链接LLVM的libunwind库
LINK_FLAGS="-arch arm64 -stdlib=libc++ -L$(xcrun --show-sdk-path)/usr/lib $LLVM20_HOME/lib/libunwind.a"

# 链接生成可执行文件
echo "链接目标文件..."
"$CLANG" $LINK_FLAGS $LLVM_LDFLAGS "${OBJECT_FILES[@]}" $LLVM_LIBS $LLVM_SYSTEM_LIBS -o miniswift

if [ $? -eq 0 ]; then
    echo "编译成功！可执行文件: $BUILD_DIR/miniswift"
    echo "运行: ./$BUILD_DIR/miniswift"
else
    echo "编译失败！"
    exit 1
fi