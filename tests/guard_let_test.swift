// Test guard let statement functionality

func testOptionalValue(value: Any?) -> String {
    guard let unwrappedValue = value else {
        return "值为nil"
    }
    return "成功解包值"
}

func testGuardLet() {
    // Test with non-nil value
    var result1 = testOptionalValue(value: 42)
    print(result1)
    
    // Test with nil value
    var result2 = testOptionalValue(value: nil)
    print(result2)
}

testGuardLet()