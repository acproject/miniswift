// 测试异步函数中的结构体操作

// 用户结构体
struct User {
    let id: Int
    let name: String
    let age: Int
}

// 处理器结构体
struct TaskProcessor {
    let processorId: String
    
    func processTask(taskId: Int, data: String) -> String {
        return "Processor[\(processorId)] processed task \(taskId) with data: \(data)"
    }
}

// 异步函数
func fetchUserData(userId: Int) async -> User {
    return User(id: userId, name: "User\(userId)", age: 25 + userId)
}

func processUserAsync(userId: Int) async -> String {
    let user = await fetchUserData(userId: userId)
    let processor = TaskProcessor(processorId: "ASYNC_PROC")
    
    let result = processor.processTask(taskId: userId, data: user.name)
    return "异步处理成功: \(result)"
}

@main
func main() {
    print("Testing async struct operations...")
    
    let asyncResult = await processUserAsync(userId: 10)
    print("\(asyncResult)")
    
    print("Test completed.")
}

main()