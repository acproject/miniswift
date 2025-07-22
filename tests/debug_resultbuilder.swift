// Simple test for @resultBuilder

@resultBuilder
struct SimpleBuilder {
    static func buildBlock(_ components: Int...) -> [Int] {
        return Array(components)
    }
}

print("Test completed")