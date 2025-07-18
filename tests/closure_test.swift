// 测试闭包功能

// 基本闭包定义和调用
let add = { (a: Int, b: Int) -> Int in
    return a + b
}

print(add)
print(add(3, 5))

// 无参数闭包
let greet = { () -> String in
    return "Hello, World!"
}

print(greet)
print(greet())

// 单参数闭包
let square = { (x: Int) -> Int in
    return x * x
}

print(square)
print(square(4))