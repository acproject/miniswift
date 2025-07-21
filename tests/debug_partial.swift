// Generic function example
func swapTwoValues<T>(a: inout T, b: inout T) {
    let temporaryA = a
    a = b
    b = temporaryA
}

// Generic struct example
struct Stack<Element> {
    var items: [Element] = []
    
    mutating func push(item: Element) {
        items.append(item)
    }
    
    mutating func pop() -> Element? {
        return items.removeLast()
    }
}

// Generic class example
class Container<T> {
    var item: T
    
    init(item: T) {
        self.item = item
    }
    
    func getItem() -> T {
        return item
    }
}

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
