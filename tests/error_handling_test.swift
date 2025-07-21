// Error Handling Test for MiniSwift

// Define a custom error type
enum NetworkError: Error {
    case timeout
    case invalidURL
    case noConnection
}

// Function that can throw an error
func fetchData() throws -> String {
    throw NetworkError.timeout
}

// Test do-catch blocks
do {
    let data = try fetchData()
    print("Data received: " + data)
} catch NetworkError.timeout {
    print("Network timeout occurred")
} catch NetworkError.invalidURL {
    print("Invalid URL provided")
} catch {
    print("Unknown error occurred")
}

// Test try? (optional try)
let optionalData = try? fetchData()
if optionalData != nil {
    print("Optional data: success")
} else {
    print("Optional data: nil")
}

// Test defer statement
func testDefer() {
    defer {
        print("This will be executed last")
    }
    
    print("This will be executed first")
    
    defer {
        print("This will be executed second to last")
    }
    
    print("This will be executed second")
}

testDefer()

print("Error handling test completed")