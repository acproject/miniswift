// Test nested types functionality

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

// Test nested types in enum
enum OuterEnum {
    case option1
    case option2
    
    struct NestedStruct {
        var enumNestedProperty: String = "enum nested"
    }
    
    class NestedClass {
        var enumClassProperty: Double = 1.41
    }
    
    enum NestedEnum {
        case inner1
        case inner2
    }
}

print("All types defined")

// Test accessing nested types
let nestedStruct = OuterStruct.NestedStruct()
print("OuterStruct.NestedStruct created")

let nestedClass = OuterClass.NestedClass()
print("OuterClass.NestedClass created")

let enumNested = OuterEnum.NestedStruct()
print("OuterEnum.NestedStruct created")

print("Nested types test completed")