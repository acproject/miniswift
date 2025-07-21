// Step 1: Basic generic function
func swapTwoValues<T>(a: inout T, b: inout T) {
    let temporaryA = a
    a = b
    b = temporaryA
}

// Step 2: Generic struct
struct Stack<Element> {
    var items: [Element] = []
    
    mutating func push(item: Element) {
        items.append(item)
    }
    
    mutating func pop() -> Element? {
        return items.removeLast()
    }
}

// Step 3: Generic class
class Container<T> {
    var item: T
    
    init(item: T) {
        self.item = item
    }
    
    func getItem() -> T {
        return item
    }
}

// Step 4: Generic function with constraints
func findIndex<T: Equatable>(of valueToFind: T, in array: [T]) -> Int? {
    for (index, value) in array.enumerated() {
        if value == valueToFind {
            return index
        }
    }
    return nil
}

// Step 5: Generic struct with where clause
struct Dictionary<Key, Value> where Key: Hashable {
    private var storage: [(Key, Value)] = []
    
    subscript(key: Key) -> Value? {
        get {
            for (k, v) in storage {
                if k == key {
                    return v
                }
            }
            return nil
        }
        set {
            for i in 0..<storage.count {
                if storage[i].0 == key {
                    if let newValue = newValue {
                        storage[i].1 = newValue
                    } else {
                        storage.remove(at: i)
                    }
                    return
                }
            }
            if let newValue = newValue {
                storage.append((key, newValue))
            }
        }
    }
}

// Usage examples
var someInt = 3
var anotherInt = 107
swapTwoValues(a: &someInt, b: &anotherInt)

var stackOfStrings = Stack<String>()
stackOfStrings.push(item: "uno")
stackOfStrings.push(item: "dos")

let stringContainer = Container<String>(item: "Hello")
let intContainer = Container<Int>(item: 42)

let strings = ["cat", "dog", "llama", "parakeet", "terrapin"]
if let foundIndex = findIndex(of: "llama", in: strings) {
    print("Found llama at index \(foundIndex)")
}