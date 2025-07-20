// Test runtime subscript functionality

struct SimpleArray {
    subscript(index: Int) -> Double {
        get {
            return 42.0
        }
        set(newValue) {
            // Simple setter for testing
        }
    }
}

struct Matrix {
    subscript(row: Int, column: Int) -> Double {
        get {
            return 1.0
        }
        set {
            // Simple setter for testing
        }
    }
}

class Dictionary {
    subscript(key: String) -> String {
        get {
            return "value"
        }
        set(newValue) {
            // Simple setter for testing
        }
    }
}

// Test single parameter subscript
let arr = SimpleArray()
let value1 = arr[0]
print("Single subscript result: ")
print(value1)

// Test multi-parameter subscript
let matrix = Matrix()
let value2 = matrix[1, 2]
print("Multi subscript result: ")
print(value2)

// Test class subscript
let dict = Dictionary()
let value3 = dict["key"]
print("Class subscript result: ")
print(value3)

print("Runtime subscript tests completed!")