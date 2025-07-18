#!/bin/bash

# MiniSwift 交互式演示脚本

echo "=== MiniSwift 交互式演示 ==="
echo "这个脚本将演示正确的 MiniSwift 语法"
echo

cd build

echo "启动 MiniSwift 交互式模式..."
echo "将执行以下命令序列："
echo "1. let name = \"Swift\""
echo "2. let greeting = \"Hello \\(name)!\""
echo "3. print(greeting)"
echo "4. let age = 25"
echo "5. let info = \"I am \\(age) years old\""
echo "6. print(info)"
echo "7. exit"
echo
echo "按 Enter 开始演示..."
read

# 创建演示输入
cat << 'EOF' | ./miniswift
let name = "Swift"
let greeting = "Hello \(name)!"
print(greeting)
let age = 25
let info = "I am \(age) years old"
print(info)
let π = 3.14159
print(π)
let 你好 = "世界"
let message = "你好，\(你好)！"
print(message)
exit
EOF

echo
echo "演示完成！"
echo
echo "常见错误示例："
echo "❌ 错误: print(s\\(n))  // 这样写是错误的"
echo "✅ 正确: let result = \"\\(s) \\(n)\"; print(result)"
echo
echo "详细语法请参考 SYNTAX_GUIDE.md 文件"