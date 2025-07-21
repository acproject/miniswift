struct Dictionary<Key, Value> where Key: Hashable {
    private var storage: [(Key, Value)] = []
    
    subscript(key: Key) -> Value? {
        get {
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
        }
    }
}