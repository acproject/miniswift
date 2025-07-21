struct Test {
    var storage: [Int] = []
    
    func test() {
        storage.insert(1, at: 0)
    }
}