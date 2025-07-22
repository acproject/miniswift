// Comprehensive test for underscore parameter labels and labeled function calls

// Function with underscore parameter labels
func multiply(_ x: Int, _ y: Int) -> Int {
    return x * y
}

// Function with mixed parameter labels (some underscore, some labeled)
func greet(_ name: String, age: Int) -> String {
    return "Hello \(name), you are \(age) years old"
}

// Function with all labeled parameters
func calculate(first: Int, second: Int, operation: String) -> Int {
    if operation == "+" {
        return first + second
    } else if operation == "*" {
        return first * second
    }
    return 0
}

// Test underscore parameters (no labels in call)
let product = multiply(6, 7)
print("Product: \(product)")

// Test mixed parameters (some labels, some not)
let greeting = greet("Alice", age: 25)
print(greeting)

// Test all labeled parameters
let sum = calculate(first: 10, second: 5, operation: "+")
print("Sum: \(sum)")

let multiplication = calculate(first: 4, second: 3, operation: "*")
print("Multiplication: \(multiplication)")