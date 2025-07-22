// 测试修复后的功能
print("=== 测试 for-in 循环和 defer 语句 ===")

// 测试 for-in 循环与范围操作符
print("\n测试 for-in 循环:")
for i in 1...3 {
    print("循环 \(i)")
}

print("\n测试半开范围:")
for j in 1..<3 {
    print("半开范围 \(j)")
}

// 测试 defer 语句
print("\n测试 defer 语句:")
func testDefer() {
    print("函数开始")
    
    defer {
        print("Defer 1: 第一个延迟执行")
    }
    
    defer {
        print("Defer 2: 第二个延迟执行")
    }
    
    print("函数中间")
    
    defer {
        print("Defer 3: 第三个延迟执行")
    }
    
    print("函数结束")
}

testDefer()

print("\n=== 测试完成 ===")