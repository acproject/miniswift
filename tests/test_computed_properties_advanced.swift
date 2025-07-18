struct Rectangle {
    var width: Double = 0.0
    var height: Double = 0.0
    
    var area: Double {
        get {
            return width * height
        }
        set {
            // 假设保持宽高比，调整宽度
            if height > 0 {
                width = newValue / height
            }
        }
    }
    
    var perimeter: Double {
        get {
            return 2 * (width + height)
        }
    }
}

var rect = Rectangle(width: 4.0, height: 3.0)
print(rect.area)      // 应该输出 12.0
print(rect.perimeter) // 应该输出 14.0

rect.area = 24.0      // 设置面积为 24，宽度应该变为 8
print(rect.width)     // 应该输出 8.0
print(rect.height)    // 应该输出 3.0 (不变)
print(rect.area)      // 应该输出 24.0
print(rect.perimeter) // 应该输出 22.0