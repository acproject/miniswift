struct Rectangle {
    var width: Double
    var height: Double
    
    var area: Double {
        get {
            return width * height
        }
        set {
            print("Setting area")
            print(width)
            width = 4.0
            print("New width:")
            print(width)
        }
    }
}

var rect = Rectangle(width: 3.0, height: 4.0)
print("Initial:")
print(rect.area)
print(rect.width)

rect.area = 20.0
print("After:")
print(rect.width)
print(rect.height)
print(rect.area)