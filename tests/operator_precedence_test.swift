// Operator Precedence Test
// Testing operator precedence group declarations

// Define custom precedence groups
precedencegroup ExponentiationPrecedence {
    associativity: right
    higherThan: MultiplicationPrecedence
}

precedencegroup CustomAdditionPrecedence {
    associativity: left
    lowerThan: MultiplicationPrecedence
    higherThan: ComparisonPrecedence
}

precedencegroup LogicalPrecedence {
    associativity: left
    lowerThan: ComparisonPrecedence
}

// Define operators with custom precedence
infix operator **: ExponentiationPrecedence
infix operator +-: CustomAdditionPrecedence
infix operator &&: LogicalPrecedence

// Implement operators
func **(base: Int, exponent: Int) -> Int {
    var result = 1
    for _ in 0..<exponent {
        result *= base
    }
    return result
}

func +-(left: Int, right: Int) -> Int {
    return left + right - 1
}

func &&(left: Bool, right: Bool) -> Bool {
    return left && right
}

print("Operator Precedence Test:")

// Test precedence: ** should have higher precedence than *
let result1 = 2 * 3 ** 2  // Should be 2 * (3 ** 2) = 2 * 9 = 18
print("2 * 3 ** 2 = \(result1)")

// Test precedence: * should have higher precedence than +-
let result2 = 2 +- 3 * 4  // Should be 2 +- (3 * 4) = 2 +- 12 = 13
print("2 +- 3 * 4 = \(result2)")

// Test associativity: ** is right-associative
let result3 = 2 ** 3 ** 2  // Should be 2 ** (3 ** 2) = 2 ** 9 = 512
print("2 ** 3 ** 2 = \(result3)")

// Test complex expression with multiple precedence levels
let result4 = 1 +- 2 * 3 ** 2  // Should be 1 +- (2 * (3 ** 2)) = 1 +- (2 * 9) = 1 +- 18 = 18
print("1 +- 2 * 3 ** 2 = \(result4)")

print("Precedence test completed successfully!")