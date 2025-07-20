struct SimpleTest {
    subscript(index: Int) -> Int {
        get {
            return index
        }
        set {
            print("Setting value")
        }
    }
}

var test = SimpleTest()
test[0] = 5
let value = test[0]
print(value)