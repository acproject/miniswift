// Test first part of original file

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

print("OuterStruct defined")