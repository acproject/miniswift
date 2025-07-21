// Generic function
func identity<T>(value: T) -> T {
    return value
}

// Generic struct
struct Container<T> {
    var item: T
    
    init(item: T) {
        self.item = item
    }
    
    func getValue() -> T {
        return item
    }
}

// Generic class
class Box<T> {
    var content: T
    
    init(content: T) {
        self.content = content
    }
    
    func getContent() -> T {
        return content
    }
}

// Generic function with constraints
func compare<T>(a: T, b: T) -> Bool where T: Equatable {
    return a == b
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
        }
    }
}

// Protocol with associated type
protocol Container2 {
    func getItem() -> Int
}

// Usage examples
let result = identity(value: 42)
let stringResult = identity(value: "Hello")

let intContainer = Container<Int>(item: 42)
let stringContainer = Container<String>(item: "World")

let intBox = Box<Int>(content: 100)
let stringBox = Box<String>(content: "Box")

let isEqual = compare(a: 5, b: 5)
let isNotEqual = compare(a: "hello", b: "world")

var dict = Dictionary<String, Int>()
dict["key1"] = 10
let value = dict["key1"]