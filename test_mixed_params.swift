// Test mixed parameter labels

func greet(_ name: String, age: Int) -> String {
    return "Hello"
}

// Test mixed call
let greeting = greet("Alice", age: 25)
print(greeting)