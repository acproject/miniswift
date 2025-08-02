// Simple Custom Operators Test
// Testing basic custom operators without complex features

// Define custom operators
infix operator **
infix operator <>
prefix operator ±

// Implement custom operators
func **(base: Int, exponent: Int) -> Int {
    var result = 1
    for _ in 0..<exponent {
        result *= base
    }
    return result
}

func <>(left: Int, right: Int) -> Int {
    return (left + right) / 2
}

prefix func ±(value: Int) -> Int {
    return value >= 0 ? value : -value
}

print("Simple Custom Operators Test:")
print("=============================")

// Test 1: Basic custom operators
print("Test 1: Custom Operators")
let power = 2 ** 3
let average = 10 <> 20
let absolute = ±(-15)
print("2 ** 3 = \(power)")
print("10 <> 20 = \(average)")
print("±(-15) = \(absolute)")

// Test 2: Operator precedence
print("Test 2: Operator Precedence")
let precedenceTest1 = 2 + 3 ** 2
let precedenceTest2 = 4 <> 6 + 2
print("2 + 3 ** 2 = \(precedenceTest1)")
print("4 <> 6 + 2 = \(precedenceTest2)")

// Test 3: Custom operators in expressions
print("Test 3: Complex Expressions")
let complexExpr1 = 2 ** 3 + 10 <> 20
let complexExpr2 = ±(-7) + 3 ** 2
print("2 ** 3 + 10 <> 20 = \(complexExpr1)")
print("±(-7) + 3 ** 2 = \(complexExpr2)")

print("Simple operators test completed successfully!")