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
    static func buildBlock(_ components: [Int]) -> [Int] {
        return components
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
        var result: [Int] = []
        for component in components {
            for item in component {
                result.append(item)
            }
        }
        return result
    }
}

// Simple function to create test values
func createTestValues() -> [Int] {
    return [1, 2, 3, 4, 5]
}

// Function to sum values
func sumValues(values: [Int]) -> Int {
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
print("±(-15) = \(absolute)\n")

// Test 2: Operator precedence
print("Test 2: Operator Precedence")
let precedenceTest1 = 2 + 3 ** 2  // Should be 2 + (3 ** 2) = 2 + 9 = 11
let precedenceTest2 = 4 <> 6 + 2  // Should be (4 <> 6) + 2 = 5 + 2 = 7
print("2 + 3 ** 2 = \(precedenceTest1)")
print("4 <> 6 + 2 = \(precedenceTest2)")
print()

// Test 3: Simple values
print("Test 3: Simple Values")
let simpleValues = createTestValues()
print("Simple values: \(simpleValues)")
print()

// Test 4: Custom operators in arrays
print("Test 4: Custom Operators in Arrays")
let complexValues = [2 ** 3, 10 <> 20, ±(-7), 3 + 4 ** 2, 5 <> 15 + 1]
print("Complex values: \(complexValues)")
print()

// Test 5: Conditional array building
print("Test 5: Conditional Array Building")
var conditionalValues = [1, 2]
if true {
    conditionalValues.append(3 ** 2)
    conditionalValues.append(4 <> 8)
}
conditionalValues.append(±(-10))
print("Conditional values: \(conditionalValues)")
print()

// Test 6: Sum calculation
print("Test 6: Sum Calculation")
let testValues = [2 ** 3, 10 <> 20, ±(-5), 3 + 2 ** 2]
let totalSum = sumValues(values: testValues)
print("Total sum: \(totalSum)")
print()

print("All advanced features test completed successfully!")