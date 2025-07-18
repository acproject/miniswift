struct Person {
    var name: String
    
    init(name: String) {
        self.name = name
        print("Person created: " + name)
    }
}