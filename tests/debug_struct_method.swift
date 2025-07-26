struct User {
    var id: Int
    var name: String
    
    func getInfo() -> String {
        return "User info"
    }
}

let user = User(id: 1, name: "Alice")
print("User created")
print(user.getInfo())