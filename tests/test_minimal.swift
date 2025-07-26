enum TaskStatus {
    case pending
    case running
    case completed
    case failed
}

struct TaskProcessor {
    let processorId: String
    
    func processTask(taskId: Int, data: String) -> String {
        if data.isEmpty {
            return "Error: Invalid input"
        }
        
        let result = "Processor[\(processorId)] processed task \(taskId) with data: \(data)"
        return result
    }
    
    func getStatus(taskId: Int) -> TaskStatus {
        if taskId < 0 {
            return TaskStatus.failed
        } else if taskId < 10 {
            return TaskStatus.completed
        } else if taskId < 20 {
            return TaskStatus.running
        } else {
            return TaskStatus.pending
        }
    }
}