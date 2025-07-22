// Compilation Test
// This test verifies that the basic Swift syntax still works correctly
// after adding new AST nodes and parser features

print("Compilation Test")
print("===============")

// Test basic variable declarations
let message = "Hello, MiniSwift!"
var counter = 0

print("Message: \(message)")
print("Counter: \(counter)")

// Test basic arithmetic
let a = 10
let b = 20
let sum = a + b
let product = a * b

print("Sum: \(sum)")
print("Product: \(product)")

// Test basic control flow
if sum > 25 {
    print("Sum is greater than 25")
} else {
    print("Sum is not greater than 25")
}

// Test basic function
func greet(name: String) -> String {
    return "Hello, " + name + "!"
}

let greeting = greet(name: "World")
print(greeting)

// Test basic struct
struct Point {
    let x: Int
    let y: Int
}

let point = Point(x: 5, y: 10)
print("Point created with x: \(point.x), y: \(point.y)")

// Test basic array
let numbers = [1, 2, 3, 4, 5]
print("Numbers: \(numbers)")

print("\n✅ All basic functionality tests passed!")
print("The compiler successfully handles existing Swift syntax.")