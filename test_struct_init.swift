struct TaskProcessor {
    let processorId: String
}

func main() {
    print("Creating processor...")
    let processor = TaskProcessor(processorId: "MAIN_PROC")
    print("Processor created successfully")
}

main()