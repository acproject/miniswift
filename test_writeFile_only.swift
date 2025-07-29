import MiniSwift

print("测试writeFile函数...")

// 测试writeFile函数
let content = "Hello, World!"
let filename = "test_output.txt"

print("正在写入文件: " + filename)
writeFile(filename, content)
print("文件写入完成!")