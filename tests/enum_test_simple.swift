// 简化的枚举测试
enum NetworkError: Error {
    case timeout
    case serverError(code: Int)
}

print("测试枚举定义")