// 测试progressive_test的后续部分

print("=== 测试Switch语句 ===")

let dayNumber = 3
switch dayNumber {
case 1:
    print("Monday")
case 2:
    print("Tuesday")
case 3:
    print("Wednesday")
case 4:
    print("Thursday")
case 5:
    print("Friday")
default:
    print("Weekend")
}

print("=== 测试结构体 ===")

struct Point {
    var x: Int
    var y: Int
    
    func getX() -> Int {
        return x
    }
    
    func getY() -> Int {
        return y
    }
}

let point = Point(x: 10, y: 20)
print("Point X: \(point.getX())")
print("Point Y: \(point.getY())")
print("Direct access - X: \(point.x), Y: \(point.y)")

print("=== 测试类 ===")

class Counter {
    var value: Int
    
    init(startValue: Int) {
        self.value = startValue
    }
    
    func getValue() -> Int {
        return value
    }
    
    func increment() {
        value = value + 1
    }
}

let counter = Counter(startValue: 0)
print("Initial counter: \(counter.getValue())")
counter.increment()
print("After increment: \(counter.getValue())")

print("=== 第二部分测试完成 ===")