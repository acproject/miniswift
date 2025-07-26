// 测试递归函数

print("=== 测试递归函数 ===")

func factorial(n: Int) -> Int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n: n - 1)
}

let fact4 = factorial(n: 4)
print("4! = \(fact4)")

print("=== 递归函数测试完成 ===")