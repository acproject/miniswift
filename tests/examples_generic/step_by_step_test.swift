// Step by step test
// Test 1: Simple function with T parameter
func test1<T>(a: T) {
    print("Test1")
}

// Test 2: Function with inout but not generic
func test2(a: inout Int) {
    print("Test2")
}