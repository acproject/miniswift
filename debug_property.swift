struct User {
    let id: Int
    let name: String
    let age: Int
}

func processUserAsync(userId: Int) async -> String {
    let user = User(id: userId, name: "TestUser", age: 25)
    print("User created successfully")
    
    let userName = user.name
    print("User name: \(userName)")
    
    return "Success"
}

@main
func main() {
    let result = await processUserAsync(userId: 42)
    print(result)
}

main()