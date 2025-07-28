// 简单的异步函数测试

// 异步函数定义
func fetchData(userId: Int) async -> String {
    return "User data for ID: \(userId)"
}

// 主函数
@main
func main() {
    print("Testing simple async function...")
    
    let result = await fetchData(userId: 42)
    print("Result: \(result)")
    
    print("Test completed.")
}

main()