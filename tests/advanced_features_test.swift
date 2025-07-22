// Advanced Features Comprehensive Test
// Testing Result Builders, Custom Operators, and Operator Precedence together

// Define custom precedence groups
precedencegroup PowerPrecedence {
    associativity: right
    higherThan: MultiplicationPrecedence
}

precedencegroup CustomCombinePrecedence {
    associativity: left
    lowerThan: MultiplicationPrecedence
    higherThan: AdditionPrecedence
}

// Define custom operators
infix operator **: PowerPrecedence
infix operator <>: CustomCombinePrecedence
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

// Define a Result Builder for mathematical expressions
@resultBuilder
struct MathBuilder {
    static func buildBlock(_ components: Int...) -> [Int] {
        return Array(components)
    }
    
    static func buildOptional(_ component: [Int]?) -> [Int] {
        return component ?? []
    }
    
    static func buildEither(first component: [Int]) -> [Int] {
        return component
    }
    
    static func buildEither(second component: [Int]) -> [Int] {
        return component
    }
    
    static func buildArray(_ components: [[Int]]) -> [Int] {
        return components.flatMap { $0 }
    }
}

// Function using the math builder
func calculateValues(@MathBuilder _ content: () -> [Int]) -> [Int] {
    return content()
}

// Function to sum values with custom operators
func sumWithCustomOps(@MathBuilder _ content: () -> [Int]) -> Int {
    let values = content()
    var sum = 0
    for value in values {
        sum += value
    }
    return sum
}

print("Advanced Features Comprehensive Test:")
print("=====================================\n")

// Test 1: Basic custom operators
print("Test 1: Custom Operators")
let power = 2 ** 3
let average = 10 <> 20
let absolute = ±(-15)
print("2 ** 3 = \(power)")
print("10 <> 20 = \(average)")
print("±(-15) = \(absolute)")
print()

// Test 2: Operator precedence
print("Test 2: Operator Precedence")
let precedenceTest1 = 2 + 3 ** 2  // Should be 2 + (3 ** 2) = 2 + 9 = 11
let precedenceTest2 = 4 <> 6 + 2  // Should be (4 <> 6) + 2 = 5 + 2 = 7
print("2 + 3 ** 2 = \(precedenceTest1)")
print("4 <> 6 + 2 = \(precedenceTest2)")
print()

// Test 3: Result Builder with simple values
print("Test 3: Result Builder")
let simpleValues = calculateValues {
    1
    2
    3
    4
    5
}
print("Simple values: \(simpleValues)")
print()

// Test 4: Result Builder with custom operators
print("Test 4: Result Builder with Custom Operators")
let complexValues = calculateValues {
    2 ** 3
    10 <> 20
    ±(-7)
    3 + 4 ** 2
    5 <> 15 + 1
}
print("Complex values: \(complexValues)")
print()

// Test 5: Conditional content in Result Builder
print("Test 5: Conditional Result Builder")
let conditionalValues = calculateValues {
    1
    2
    if true {
        3 ** 2
        4 <> 8
    }
    ±(-10)
}
print("Conditional values: \(conditionalValues)")
print()

// Test 6: Sum calculation using Result Builder
print("Test 6: Sum with Result Builder")
let totalSum = sumWithCustomOps {
    2 ** 3      // 8
    10 <> 20    // 15
    ±(-5)       // 5
    3 + 2 ** 2  // 7
}
print("Total sum: \(totalSum)")
print()

print("All advanced features test completed successfully!")