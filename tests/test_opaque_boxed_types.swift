// Test file for opaque types (some) and boxed protocol types (any)

// Define test protocols
protocol TestProtocol {
    
}

protocol AnotherProtocol {
    
}

// Test 1: Basic opaque type usage
let opaqueValue: some TestProtocol = 42
print("Opaque value created")

// Test 2: Basic boxed protocol type usage
let boxedValue: any TestProtocol = "Hello"
print("Boxed value created")

// Test 3: Different protocols
let anotherOpaque: some AnotherProtocol = 3.14
print("Another opaque value created")

let anotherBoxed: any AnotherProtocol = true
print("Another boxed value created")

// Test 4: Function with opaque return type
func makeOpaque() -> some TestProtocol {
    return 42
}

let result = makeOpaque()
print("Function with opaque return type works")

print("All opaque and boxed type tests completed successfully!")