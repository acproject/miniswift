enum TaskStatus {
    case pending
    case running
    case completed
    case failed
}

func main() {
    let status = TaskStatus.completed
    print("Status: \(status)")
}

main()