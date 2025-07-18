#!/bin/bash

# 错误恢复测试脚本

echo "=== MiniSwift 错误恢复测试 ==="
echo "测试程序在遇到错误后是否能继续执行后续语句"
echo

cd build

echo "测试场景 1: 字符串插值语法错误后继续执行"
echo "输入序列："
echo "1. let name = \"Swift\""
echo "2. let bad = \"\\(name) \\(age)\"  // 错误：age 未定义"
echo "3. print(\"This should still work\")  // 应该正常执行"
echo "4. print(name)  // 应该正常执行"
echo "5. exit"
echo

cat << 'EOF' | ./miniswift
let name = "Swift"
let bad = "\(name) \(age)"
print("This should still work")
print(name)
let another_bad = "\(undefined_var)"
print("Still working after second error")
let good = "Hello World"
print(good)
exit
EOF

echo
echo "测试场景 2: 语法错误混合测试"
echo

cat << 'EOF' | ./miniswift
let a = 10
invalid syntax here
print("After syntax error")
let b = 20
print(b)
let bad_interpolation = "\(nonexistent)"
print("After interpolation error")
print(a)
exit
EOF

echo
echo "=== 错误恢复测试完成 ==="
echo "如果看到多个 'After ... error' 消息，说明错误恢复机制工作正常"