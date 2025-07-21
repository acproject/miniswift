// Error Handling Test for MiniSwift

// Define a custom error type
enum NetworkError: Error {
    case timeout
    case invalidURL
    case noConnection
}

// Function that can throw an error
func fetchData() throws -> String {
    // Simulate a network error
    throw NetworkError.timeout
}

// Function that returns a Result type
func safeParseInt(_ str: String) -> Result<Int, ParseError> {
    if str == "42" {
        return Result.success(42)
    } else {
        return Result.failure(ParseError.invalidFormat)
    }
}

// Test do-catch blocks
do {
    let data = try fetchData()
    print("Data received: \(data)")
} catch NetworkError.timeout {
    print("Network timeout occurred")
} catch NetworkError.invalidURL {
    print("Invalid URL provided")
} catch {
    print("Unknown error: \(error)")
}

// Test try? (optional try)
let optionalData = try? fetchData()
print("Optional data: \(optionalData)")

// Test Result type
let result1 = safeParseInt("42")
print("Parse result 1: \(result1)")

let result2 = safeParseInt("invalid")
print("Parse result 2: \(result2)")

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

// Test guard statement
func processValue(_ value: Int?) {
    guard let unwrappedValue = value else {
        print("Value is nil, returning early")
        return
    }
    
    print("Processing value: \(unwrappedValue)")
}

processValue(42)
processValue(nil)