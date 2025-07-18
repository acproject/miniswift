// Comprehensive function test

// Simple function without parameters
func greet() {
    print("Hello, World!");
}

// Function with parameters and return type
func add(a: Int, b: Int) -> Int {
    return a + b;
}

// Function with string operations
func sayHello(name: String) -> String {
    return "Hello, " + name + "!";
}

// Function that returns early
func checkNumber(num: Int) -> String {
    if (num > 0) {
        return "positive";
    }
    return "non-positive";
}

// Test function calls
greet();

let result = add(5, 3);
print(result);

let greeting = sayHello("Alice");
print(greeting);

let check1 = checkNumber(10);
print(check1);

let check2 = checkNumber(-5);
print(check2);