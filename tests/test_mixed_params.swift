func greet(_ name: String, age: Int) {
    print("Hello \(name), you are \(age) years old!")
}

// Test mixed parameters: first parameter has no label, second has label
greet("Alice", age: 25)