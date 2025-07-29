// 测试文件写入功能
print("开始测试文件写入功能...")

// 尝试调用writeFile函数
var content = "Hello, MiniSwift! This is a test file."
var filePath = "/tmp/test_miniswift.txt"

print("准备写入内容: \(content)")
print("目标文件路径: \(filePath)")

// 尝试调用writeFile函数（如果已注册）
writeFile(filePath, content)

print("文件写入测试完成")