// Recursive function test

// Factorial function
func factorial(n: Int) -> Int {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

// Fibonacci function
func fibonacci(n: Int) -> Int {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// Test recursive functions
print("Factorial of 5:");
print(factorial(5));

print("Fibonacci of 6:");
print(fibonacci(6));