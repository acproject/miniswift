// Simple ternary operator test

print("Testing ternary operator:")

// Basic ternary test
let a = 5
let b = 10
let max = a > b ? a : b
print("max of \(a) and \(b) is \(max)")

// Nested ternary test
let x = 3
let y = 7
let z = 2
let result = x > y ? x : (y > z ? y : z)
print("max of \(x), \(y), and \(z) is \(result)")

// Ternary with expressions
let condition = true
let value = condition ? 42 + 8 : 10 * 3
print("conditional value: \(value)")

print("Ternary operator test completed!")