// Advanced Operators Test File
// This file tests the newly implemented advanced operators in MiniSwift

// Test bitwise operators
let a = 5    // Binary: 101
let b = 3    // Binary: 011

print("Bitwise AND: \(a & b)")    // Should print 1 (001)
print("Bitwise OR: \(a | b)")     // Should print 7 (111)
print("Bitwise XOR: \(a ^ b)")    // Should print 6 (110)
print("Bitwise NOT: \(~a)")       // Should print -6

// Test bitwise shift operators
let x = 8    // Binary: 1000
print("Left shift: \(x << 2)")    // Should print 32 (100000)
print("Right shift: \(x >> 2)")   // Should print 2 (10)

// Test overflow arithmetic operators
let maxInt = 2147483647  // Maximum 32-bit signed integer
let y = maxInt &+ 1      // Overflow addition
print("Overflow addition: \(y)")

let z = 10 &- 15         // Overflow subtraction
print("Overflow subtraction: \(z)")

let w = 1000 &* 1000000  // Overflow multiplication
print("Overflow multiplication: \(w)")

// Test custom operator declaration
operator prefix +++
operator infix *** : MultiplicationPrecedence
operator postfix !!!

// Test precedence group declaration
precedencegroup CustomPrecedence {
    associativity: left
    precedence: 150
}

// Test Result Builder declaration
@resultBuilder
struct ArrayBuilder {
    static func buildBlock(_ components: Int...) -> [Int] {
        return Array(components)
    }
}

// Test function using result builder
func buildArray(@ArrayBuilder builder: () -> [Int]) -> [Int] {
    return builder()
}

let numbers = buildArray {
    1
    2
    3
    4
}

print("Built array: \(numbers)")

// Test complex expressions with multiple operators
let result1 = (a | b) & (x >> 1)
print("Complex bitwise: \(result1)")

let result2 = (maxInt &+ 1) &* 2
print("Complex overflow: \(result2)")

print("Advanced operators test completed!")