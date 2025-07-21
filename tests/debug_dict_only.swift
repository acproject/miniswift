// Test Dictionary struct with where clause
struct Dictionary<Key, Value> where Key: Hashable {
    private var storage: [(Key, Value)] = []
}