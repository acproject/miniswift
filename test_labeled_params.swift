// Test file for underscore parameter labels and labeled function calls

// Function with underscore parameter labels
func power(_ base: Int, _ exponent: Int) -> Int {
    var result = 1
    for i in 1...exponent {
        result = result * base
    }
    return result
}

// Function with labeled parameters
func average(a: Int, b: Int) -> Int {
    return (a + b) / 2
}

// Test the functions
let powerResult = power(2, 3)  // Should work with underscore labels
print("Power result: \(powerResult)")

let averageResult = average(a: 10, b: 20)  // Should work with labeled call
print("Average result: \(averageResult)")