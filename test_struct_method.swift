enum TaskStatus {
    case pending
    case running
    case completed
    case failed
}

struct TaskProcessor {
    let processorId: String
    
    func processTask(taskId: Int, data: String) -> String {
        return "Processor[\(processorId)] processed task \(taskId) with data: \(data)"
    }
    
    func getStatus(taskId: Int) -> TaskStatus {
        if taskId < 10 {
            return TaskStatus.completed
        } else {
            return TaskStatus.pending
        }
    }
}

func main() {
    print("Creating processor...")
    let processor = TaskProcessor(processorId: "MAIN_PROC")
    print("Processor created")
    
    print("Calling processTask...")
    let result1 = processor.processTask(taskId: 1, data: "测试数据")
    print("Result: \(result1)")
    
    print("Calling getStatus...")
    let status = processor.getStatus(taskId: 5)
    print("Status: \(status)")
}

main()