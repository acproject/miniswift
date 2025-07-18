struct Person {
    var name: String
    
    init(name: String) {
        self.name = name
    }
    
    deinit {
        print("Person destroyed")
    }
}