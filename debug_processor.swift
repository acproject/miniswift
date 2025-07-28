struct User {
    let id: Int
    let name: String
    let age: Int
}

struct TaskProcessor {
    let processorId: String
    
    func processTask(taskId: Int, data: String) -> String {
        return "Processor[\(processorId)] processed task \(taskId) with data: \(data)"
    }
}

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
    let asyncResult = await processUserAsync(userId: 10)
    print(asyncResult)
}

main()