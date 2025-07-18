// 基本枚举测试
enum Direction {
    case north
    case south
    case east
    case west
}

// 使用枚举
let heading = Direction.north
print(heading)

// 带原始值的枚举
enum Planet: Int {
    case mercury = 1
    case venus = 2
    case earth = 3
    case mars = 4
}

let earthOrder = Planet.earth
print(earthOrder)
print(earthOrder.rawValue)

// 带关联值的枚举
enum Barcode {
    case upc(Int, Int, Int, Int)
    case qrCode(String)
}

let productBarcode = Barcode.upc(8, 85909, 51226, 3)
print(productBarcode)

let qrBarcode = Barcode.qrCode("ABCDEFGHIJKLMNOP")
print(qrBarcode)

// Switch 语句匹配枚举
switch heading {
case .north:
    print("Lots of planets have a north")
case .south:
    print("Watch out for penguins")
case .east:
    print("Where the sun rises")
case .west:
    print("Where the skies are blue")
}