#!/bin/bash

# MiniSwift Playground 测试脚本

echo "=== 编译 MiniSwift ==="
cd build
make

if [ $? -eq 0 ]; then
    echo "\n=== 运行 Playground ==="
    echo "执行文件: playground.swift"
    echo "----------------------------------------"
    cat ../playground.swift | ./miniswift
    echo "\n----------------------------------------"
    echo "测试完成！"
else
    echo "编译失败！"
    exit 1
fi