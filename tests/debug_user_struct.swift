// 调试User结构体方法问题

struct User {
    let id: Int
    let name: String
    var age: Int
    
    init(id: Int, name: String, age: Int) {
        self.id = id
        self.name = name
        self.age = age
    }
    
    func getInfo() -> String {
        return "User(id: \(id), name: \(name), age: \(age))"
    }
}

@main
func main() {
    print("测试User结构体方法:")
    let user1 = User(id: 1, name: "张三", age: 25)
    print("调用getInfo方法:")
    let info = user1.getInfo()
    print("结果: \(info)")
}

main()