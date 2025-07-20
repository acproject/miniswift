// Comprehensive subscript test

// Test 1: Basic struct with subscript
struct SimpleArray {
    subscript(index: Int) -> Int {
        get {
            return index * 2
        }
        set {
            print("Setting value at index " + index + " to " + newValue)
        }
    }
}

// Test 2: Class with subscript
class Dictionary {
    subscript(key: String) -> String {
        get {
            return "value for " + key
        }
        set {
            print("Setting " + key + " to " + newValue)
        }
    }
}

// Test 3: Multiple parameter subscript
struct Matrix {
    subscript(row: Int, col: Int) -> Int {
        get {
            return row + col
        }
        set(value) {
            print("Setting matrix[" + row + "][" + col + "] to " + value)
        }
    }
}

print("All subscript declarations parsed successfully!")