# Error Handling Implementation in MiniSwift

## Overview

This document describes the implementation of error handling features in MiniSwift, following Swift's error handling model. The implementation includes the Error protocol, throw/try/catch syntax, Result type, and error propagation mechanisms.

## Architecture

### Core Components

1. **ErrorHandling.h** - Foundational error handling architecture
2. **ErrorNodes.h** - AST nodes for error handling syntax
3. **Value.h** - Extended to support Error and Result types
4. **Interpreter** - Extended with error handling visitor methods

### Error Protocol

```cpp
class ErrorProtocol {
public:
    virtual ~ErrorProtocol() = default;
    virtual std::string getErrorType() const = 0;
    virtual std::string getMessage() const = 0;
    virtual std::unordered_map<std::string, Value> getAdditionalInfo() const = 0;
};
```

### Built-in Error Types

- **RuntimeError** - General runtime errors
- **TypeError** - Type-related errors
- **IndexOutOfBoundsError** - Array/collection access errors
- **KeyNotFoundError** - Dictionary key access errors

### Result Type

```cpp
template<typename T>
struct Result {
    bool isSuccess;
    T value;
    ErrorValue error;
    
    static Result success(const T& val);
    static Result failure(const ErrorValue& err);
};
```

## Syntax Support

### Throw Statement

```swift
throw NetworkError.timeout
throw CustomError("Something went wrong")
```

### Do-Catch Blocks

```swift
do {
    let result = try riskyOperation()
    print("Success: \(result)")
} catch NetworkError.timeout {
    print("Network timeout")
} catch let error as CustomError {
    print("Custom error: \(error.message)")
} catch {
    print("Unknown error: \(error)")
}
```

### Try Expressions

- **try** - Normal try, propagates errors
- **try?** - Optional try, converts errors to nil
- **try!** - Forced try, crashes on error

```swift
let result1 = try riskyOperation()        // Propagates error
let result2 = try? riskyOperation()       // Returns nil on error
let result3 = try! riskyOperation()       // Crashes on error
```

### Guard Statements

```swift
guard let value = optionalValue else {
    print("Value is nil")
    return
}
```

### Defer Statements

```swift
func processFile() {
    defer {
        print("Cleanup code")
    }
    
    // Main processing code
    // Defer block executes when function exits
}
```

## AST Nodes

### Statement Nodes

- **ThrowStmt** - `throw expression`
- **DoCatchStmt** - `do { } catch { }`
- **DeferStmt** - `defer { }`
- **GuardStmt** - `guard condition else { }`

### Expression Nodes

- **TryExpr** - `try`, `try?`, `try!`
- **ResultTypeExpr** - `Result<Success, Error>`
- **ErrorLiteral** - Error instance creation

## Error Context Management

### ErrorContext

```cpp
struct ErrorContext {
    std::set<std::string> catchableErrorTypes;
    bool catchesAllErrors = false;
    bool isInTryBlock = false;
};
```

### Error Propagation

The `ErrorPropagator` class manages how errors propagate through the call stack:

- Checks if errors can be caught in current context
- Determines propagation behavior based on try expression type
- Manages error context stack for nested try-catch blocks

## Implementation Details

### Interpreter Extensions

The `Interpreter` class has been extended with:

- Error context stack management
- Deferred statement execution
- Error handling visitor methods
- Helper methods for error propagation

### Value System Integration

The `Value` system now supports:

- `ValueType::Error` for error values
- `ValueType::Result` for Result type values
- Proper equality comparison for error types
- Print support for error and result values

### Exception Handling

The implementation uses a custom `ThrowException` class that extends `std::runtime_error` to handle Swift-style error throwing while maintaining compatibility with C++ exception handling.

## Usage Examples

### Basic Error Handling

```swift
enum FileError: Error {
    case notFound
    case permissionDenied
    case corrupted
}

func readFile(_ path: String) throws -> String {
    if path.isEmpty {
        throw FileError.notFound
    }
    return "File contents"
}

do {
    let content = try readFile("example.txt")
    print(content)
} catch FileError.notFound {
    print("File not found")
} catch {
    print("Other error: \(error)")
}
```

### Result Type Usage

```swift
func divide(_ a: Double, _ b: Double) -> Result<Double, MathError> {
    if b == 0 {
        return Result.failure(MathError.divisionByZero)
    }
    return Result.success(a / b)
}

let result = divide(10, 2)
switch result {
case .success(let value):
    print("Result: \(value)")
case .failure(let error):
    print("Error: \(error)")
}
```

### Defer and Guard

```swift
func processData(_ data: Data?) {
    defer {
        print("Cleanup completed")
    }
    
    guard let validData = data else {
        print("Invalid data")
        return
    }
    
    // Process valid data
    print("Processing: \(validData)")
}
```

## Testing

A comprehensive test file `tests/error_handling_test.swift` demonstrates all error handling features:

- Custom error types
- Throwing functions
- Do-catch blocks
- Try expressions (try, try?, try!)
- Result type usage
- Defer statements
- Guard statements

## Future Enhancements

1. **Error Chaining** - Support for error cause chains
2. **Async Error Handling** - Integration with async/await
3. **Error Recovery** - Advanced error recovery mechanisms
4. **Performance Optimization** - Optimize error handling performance
5. **Debugging Support** - Enhanced error debugging information

## Conclusion

The error handling implementation provides a robust foundation for Swift-style error management in MiniSwift. It supports all major error handling patterns while maintaining compatibility with the existing interpreter architecture.