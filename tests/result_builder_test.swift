// Result Builder Test
// Testing the @resultBuilder functionality

@resultBuilder
struct ArrayBuilder {
    static func buildBlock(_ components: Int...) -> [Int] {
        return Array(components)
    }
    
    static func buildOptional(_ component: [Int]?) -> [Int] {
        return component ?? []
    }
    
    static func buildEither(first component: [Int]) -> [Int] {
        return component
    }
    
    static func buildEither(second component: [Int]) -> [Int] {
        return component
    }
}

// Function using the result builder
func buildArray(@ArrayBuilder _ content: () -> [Int]) -> [Int] {
    return content()
}

// Test the result builder
let numbers = buildArray {
    1
    2
    3
    4
    5
}

print("Result Builder Test:")
print("Built array: \(numbers)")
print("Array count: \(numbers.count)")

// Test with conditional content
let conditionalNumbers = buildArray {
    1
    2
    if true {
        3
        4
    }
    5
}

print("Conditional array: \(conditionalNumbers)")