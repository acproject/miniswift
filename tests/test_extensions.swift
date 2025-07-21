// Extension功能基础测试文件

// 定义一个基础结构体
struct Point {
    var x: Int
    var y: Int
    
    init(x: Int, y: Int) {
        self.x = x
        self.y = y
    }
}

// 为Point添加扩展
extension Point {
    // 扩展方法
    func distance(self: Point) -> Int {
        return self.x * self.x + self.y * self.y
    }
}

// 测试代码
print("=== Extension测试开始 ===")

// 测试Point扩展
var point1 = Point()
point1.x = 3
point1.y = 4
print("Point1 created")
print("Distance squared: ")
print(point1.distance())

print("=== Extension测试结束 ===")