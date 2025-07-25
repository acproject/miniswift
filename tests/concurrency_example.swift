// MiniSwift 并发关键字识别测试
// 测试词法分析器对并发关键字的识别

// 测试 Actor 声明
actor TestActor {
    var data: Int = 42
}

// 测试函数中的 await 关键字
func testAwait() {
    print("测试 await 关键字")
    await 42
}

// 测试 Task 关键字
func testTask() {
    print("测试 Task 关键字")
    
    Task {
        42
    }
}

// 主函数
func main() {
    print("=== 并发关键字测试 ===")
    
    testAwait()
    testTask()
    
    print("=== 测试完成 ===")
}

main()