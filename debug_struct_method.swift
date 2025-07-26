struct User {
    var name: String
    var age: Int
    
    func getInfo() -> String {
        return "Name: \(name), Age: \(age)"
    }
}

let user = User(name: "Alice", age: 30)
print(user.getInfo())