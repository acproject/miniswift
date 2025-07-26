// 测试增强的并发特性
// 包括 TaskGroup、AsyncSequence、AsyncLet 和 for-await-in 循环

// 测试 Actor 定义
actor DataProcessor {
    private var data: [Int] = []
    
    func addData(_ value: Int) {
        data.append(value)
    }
    
    func processData() async -> [Int] {
        return data.map { $0 * 2 }
    }
}

// 测试异步函数
func fetchData() async -> [Int] {
    // 模拟异步数据获取
    return [1, 2, 3, 4, 5]
}

// 测试 TaskGroup 操作
func processDataWithTaskGroup() async -> [Int] {
    return await withTaskGroup(of: Int.self) { group in
        var results: [Int] = []
        
        // 添加多个任务到组
        for i in 1...5 {
            group.addTask {
                return await computeValue(i)
            }
        }
        
        // 等待所有任务完成
        for await result in group {
            results.append(result)
        }
        
        return results
    }
}

// 测试 async let 绑定
func processWithAsyncLet() async -> (Int, String) {
    async let value = computeValue(10)
    async let text = fetchText()
    
    return await (value, text)
}

// 测试 AsyncSequence
struct NumberSequence: AsyncSequence {
    typealias Element = Int
    
    func makeAsyncIterator() -> AsyncIterator {
        return AsyncIterator()
    }
    
    struct AsyncIterator: AsyncIteratorProtocol {
        private var current = 0
        
        mutating func next() async -> Int? {
            guard current < 10 else { return nil }
            current += 1
            return current
        }
    }
}

// 测试 for-await-in 循环
func processAsyncSequence() async -> [Int] {
    var results: [Int] = []
    let sequence = NumberSequence()
    
    for await number in sequence {
        results.append(number * 2)
    }
    
    return results
}

// 辅助函数
func computeValue(_ input: Int) async -> Int {
    // 模拟异步计算
    return input * input
}

func fetchText() async -> String {
    return "Hello, Async!"
}

// 主函数测试
@main
struct ConcurrencyTest {
    static func main() async {
        print("Testing enhanced concurrency features...")
        
        // 测试 Actor
        let processor = DataProcessor()
        await processor.addData(42)
        let processedData = await processor.processData()
        print("Processed data: \(processedData)")
        
        // 测试 TaskGroup
        let groupResults = await processDataWithTaskGroup()
        print("TaskGroup results: \(groupResults)")
        
        // 测试 async let
        let (value, text) = await processWithAsyncLet()
        print("Async let results: \(value), \(text)")
        
        // 测试 AsyncSequence
        let sequenceResults = await processAsyncSequence()
        print("AsyncSequence results: \(sequenceResults)")
        
        print("All concurrency tests completed!")
    }
}