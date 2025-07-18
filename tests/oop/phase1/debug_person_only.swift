struct Person {
    var name: String
    var age: Int
    
    init(name: String, age: Int) {
        self.name = name
        self.age = age
        print("Person created: " + name)
    }
    
    deinit {
        print("Person destroyed: " + self.name)
    }
}