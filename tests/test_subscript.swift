// Test subscript functionality

struct Matrix {
    var data: Int
    var rows: Int
    var cols: Int
    
    subscript(row: Int, col: Int) -> Int {
        get {
            return data  // Simplified for testing
        }
        set {
            data = newValue  // Simplified for testing
        }
    }
}

class Dictionary {
    var value: Int
    
    subscript(key: String) -> Int {
        get {
            return value
        }
        set {
            value = newValue
        }
    }
}

// Test the subscript functionality
struct SimpleStruct {
    var value: Int
    
    subscript(index: Int) -> Int {
        get {
            return value
        }
        set {
            value = newValue
        }
    }
}

var simple = SimpleStruct()
simple.value = 42
print("Testing simple subscript...")
print(simple[0])
simple[1] = 5
print(simple[1])

var dict = Dictionary()
dict.value = 10
print("Testing dictionary subscript...")
print(dict["test"])
dict["key"] = 20
print(dict["key"])