// Generic function with constraints
func findIndex<T: Equatable>(of valueToFind: T, in array: [T]) -> Int? {
    for (index, value) in array.enumerated() {
        if value == valueToFind {
            return index
        }
    }
    return nil
}

// Generic struct with where clause
struct Dictionary<Key, Value> where Key: Hashable {
    private var storage: [(Key, Value)] = []
}

// Usage that causes the error
let strings = ["cat", "dog", "llama", "parakeet", "terrapin"]
if let foundIndex = findIndex(of: "llama", in: strings) {
    print("Found llama at index \(foundIndex)")
}