#!/bin/bash

echo "测试交互模式下的转义字符处理..."

# 创建临时输入文件
cat > /tmp/miniswift_input.txt << 'EOF'
print("Hello\nWorld");
var message = "Tab:\tTest";
print(message);
var quote = "She said \"Hi\"";
print(quote);
exit
EOF

echo "运行交互模式测试:"
cd /Users/acproject/workspace/cpp_projects/MiniSwift
./build/miniswift < /tmp/miniswift_input.txt

echo "\n测试完成！"
rm -f /tmp/miniswift_input.txt