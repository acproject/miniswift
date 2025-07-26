// 测试带标签的结构体初始化语法

struct User {
    let id: Int
    let name: String
    var age: Int
}
@main
func main() {
    // 测试带标签的结构体初始化
    let user1 = User(id: 1, name: "Alice", age: 30)
    let user2 = User(id: 2, name: "Bob", age: 35)
    
    print("User 1: id=\(user1.id), name=\(user1.name), age=\(user1.age)")
    print("User 2: id=\(user2.id), name=\(user2.name), age=\(user2.age)")
}
main()