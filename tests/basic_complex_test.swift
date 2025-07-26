// 数据结构定义
struct User {
    let id: Int
    let name: String
    var age: Int
    
    func getInfo() -> String {
        return "User(id: \(id), name: \(name), age: \(age))"
    }
    
    mutating func incrementAge() {
        age = age + 1
    }
}


let user2 = User(id: 2, name: "Bob", age: 35)

print("User 2: id=\(user2.id), name=\(user2.name), age=\(user2.age)")

print("User 2: \(user2.getInfo())")