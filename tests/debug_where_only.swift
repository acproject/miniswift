struct Dictionary<Key, Value> where Key: Hashable {
    var storage: [(Key, Value)] = []
}