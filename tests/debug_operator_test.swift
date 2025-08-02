// Debug test for custom operators

// Define custom operator
infix operator **

// Implement custom operator function
func **(base: Int, exponent: Int) -> Int {
    print("Inside ** function with base: \(base), exponent: \(exponent)")
    return base * base  // Simple implementation for testing
}

// Test direct function call
print("Testing direct function call:")
let directResult = **(2, 3)
print("Direct call result: \(directResult)")

// Test operator usage
print("Testing operator usage:")
let operatorResult = 2 ** 3
print("Operator usage result: \(operatorResult)")