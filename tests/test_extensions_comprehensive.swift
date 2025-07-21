// 综合扩展测试

// 定义基础结构体
struct Point {
    var x: Int
    var y: Int
}

// 为Point添加距离计算扩展
extension Point {
    func distance() -> Int {
        return self.x * self.x + self.y * self.y
    }
    
    func distanceFrom(other: Point) -> Int {
        let dx = self.x - other.x
        let dy = self.y - other.y
        return dx * dx + dy * dy
    }
}

// 定义另一个结构体
struct Rectangle {
    var width: Int
    var height: Int
}

// 为Rectangle添加面积计算扩展
extension Rectangle {
    func area() -> Int {
        return self.width * self.height
    }
    
    func perimeter() -> Int {
        return 2 * (self.width + self.height)
    }
}

print("=== 综合扩展测试开始 ===")

// 测试Point扩展
var point1 = Point
point1.x = 3
point1.y = 4

var point2 = Point
point2.x = 6
point2.y = 8

print("Point1 distance squared:")
print(point1.distance())

print("Point2 distance squared:")
print(point2.distance())

// print("Distance between points:")
// print(point1.distanceFrom(other: point2))

// 测试Rectangle扩展
var rect = Rectangle
rect.width = 5
rect.height = 3

print("Rectangle area:")
print(rect.area())

print("Rectangle perimeter:")
print(rect.perimeter())

print("=== 综合扩展测试结束 ===")