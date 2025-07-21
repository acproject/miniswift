// Simple test for tuple member access
struct TestStruct {
    func test() {
        let tuple = ("hello", 42)
        let first = tuple.0
        let second = tuple.1
        print(first)
        print(second)
    }
}

var test = TestStruct()
test.test()