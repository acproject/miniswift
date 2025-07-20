// Simple subscript test

struct TestStruct {
    var value: Int
    
    init(value: Int) {
        self.value = value
    }
    
    subscript(index: Int) -> Int {
        get {
            return value
        }
        set {
            value = newValue
        }
    }
}

var test = TestStruct(value: 42)
print("Initial value:")
print(test[0])
test[1] = 100
print("After setting:")
print(test[1])