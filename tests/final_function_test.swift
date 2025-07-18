// Final comprehensive function test
// Testing all function features together

// Global variables
var globalCounter: Int = 0;

// Function that modifies global state
func incrementCounter() {
    globalCounter = globalCounter + 1;
}

// Function with multiple parameters and complex logic
func processNumbers(x: Int, y: Int, operation: String) -> Int {
    if (operation == "add") {
        return x + y;
    }
    if (operation == "multiply") {
        return x * y;
    }
    return 0;
}

// Function that works with arrays
func sumArray(arr: Array) -> Int {
    var sum: Int = 0;
    var i: Int = 0;
    while (i < 3) {
        sum = sum + arr[i];
        i = i + 1;
    }
    return sum;
}

// Function composition test
func double(n: Int) -> Int {
    return n * 2;
}

func addTen(n: Int) -> Int {
    return n + 10;
}

// Main test execution
print("=== Function Integration Test ===");

// Test global state modification
print("Initial counter:");
print(globalCounter);
incrementCounter();
print("After increment:");
print(globalCounter);

// Test complex function logic
let addResult = processNumbers(5, 3, "add");
let multiplyResult = processNumbers(4, 6, "multiply");
print("5 + 3 =");
print(addResult);
print("4 * 6 =");
print(multiplyResult);

// Test array processing
let numbers = [1, 2, 3];
let arraySum = sumArray(numbers);
print("Sum of [1, 2, 3]:");
print(arraySum);

// Test function composition
let composed = addTen(double(5));
print("addTen(double(5)):");
print(composed);

print("=== All tests completed! ===");