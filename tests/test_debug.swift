enum TaskStatus {
    case pending
    case running
    case completed
    case failed
}

struct TaskProcessor {
    let processorId: String
    
    func getStatus(taskId: Int) -> TaskStatus {
        if taskId < 0 {
        }
        return TaskStatus.pending
    }
}
