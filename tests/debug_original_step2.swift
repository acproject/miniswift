// Test first two parts of original file

// Test nested types in struct
struct OuterStruct {
    var outerProperty: Int = 10
    
    struct NestedStruct {
        var nestedProperty: String = "nested"
    }
    
    class NestedClass {
        var classProperty: Double = 3.14
    }
    
    enum NestedEnum {
        case first
        case second
    }
}

// Test nested types in class
class OuterClass {
    var outerProperty: Int = 20
    
    struct NestedStruct {
        var nestedProperty: String = "class nested"
    }
    
    class NestedClass {
        var classProperty: Double = 2.71
    }
    
    enum NestedEnum {
        case alpha
        case beta
    }
}

print("OuterStruct and OuterClass defined")