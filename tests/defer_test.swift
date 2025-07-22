// 专门测试 defer 语句
print("=== Defer 语句专项测试 ===")

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

func testDeferWithReturn() {
    print("\n=== 测试带return的defer ===")
    
    defer {
        print("Defer: 即使有return也会执行")
    }
    
    print("函数开始")
    return
}

testDeferWithReturn()

print("\n=== 测试完成 ===")