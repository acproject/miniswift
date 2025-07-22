// Simple Features Test
// Testing basic functionality and nil-coalescing operator

// Helper functions
func power(_ base: Int, _ exponent: Int) -> Int {
    var result = 1
    for _ in 0..<exponent {
        result *= base
    }
    return result
}

func average(_ left: Int, _ right: Int) -> Int {
    return (left + right) / 2
}

func absolute(_ value: Int) -> Int {
    return value >= 0 ? value : -value
}

print("Simple Features Test:")
print("===================\n")

// Test 1: Basic arithmetic
print("Test 1: Basic Arithmetic")
let powerResult = power(2, 3)
let averageResult = average(10, 20)
let absoluteResult = absolute(-15)
print("2^3 = \(powerResult)")
print("average(10, 20) = \(averageResult)")
print("absolute(-15) = \(absoluteResult)")
print()

// Test 2: Nil-coalescing operator
print("Test 2: Nil-Coalescing Operator")
let optionalValue: Int? = nil
let defaultValue = 42
let result = optionalValue ?? defaultValue
print("nil ?? 42 = \(result)")

let nonNilValue: Int? = 10
let result2 = nonNilValue ?? defaultValue
print("10 ?? 42 = \(result2)")
print()

// Test 3: Arrays and basic operations
print("Test 3: Arrays")
let numbers = [1, 2, 3, 4, 5]
print("Numbers: \(numbers)")

var sum = 0
for number in numbers {
    sum += number
}
print("Sum: \(sum)")
print()

// Test 4: String interpolation
print("Test 4: String Interpolation")
let name = "Alice"
let age = 25
print("Name: \(name)")
print("\(name) is \(age) years old")
print()

print("All tests completed successfully!")