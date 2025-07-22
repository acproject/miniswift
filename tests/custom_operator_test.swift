// Custom Operator Test
// Testing custom operator declarations and usage

// Define custom operators
infix operator **: MultiplicationPrecedence
infix operator +-: AdditionPrecedence
prefix operator √
postfix operator !

// Implement the power operator
func **(base: Int, exponent: Int) -> Int {
    var result = 1
    for _ in 0..<exponent {
        result *= base
    }
    return result
}

// Implement the plus-minus operator
func +-(left: Int, right: Int) -> Int {
    return left + right - 1
}

// Implement square root operator (simplified)
prefix func √(value: Int) -> Int {
    var result = 0
    while result * result < value {
        result += 1
    }
    return result
}

// Implement factorial operator
postfix func !(value: Int) -> Int {
    if value <= 1 {
        return 1
    }
    var result = 1
    for i in 2...value {
        result *= i
    }
    return result
}

print("Custom Operator Test:")

// Test power operator
let powerResult = 2 ** 3
print("2 ** 3 = \(powerResult)")

// Test plus-minus operator
let plusMinusResult = 5 +- 3
print("5 +- 3 = \(plusMinusResult)")

// Test square root operator
let sqrtResult = √16
print("√16 = \(sqrtResult)")

// Test factorial operator
let factorialResult = 5!
print("5! = \(factorialResult)")

// Test operator precedence
let complexExpression = 2 ** 3 + 4 * 5
print("2 ** 3 + 4 * 5 = \(complexExpression)")