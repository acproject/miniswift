// Test step by step

// First define OuterStruct
struct OuterStruct {
    var outerProperty: Int = 10
    
    struct NestedStruct {
        var nestedProperty: String = "nested"
    }
}

print("OuterStruct defined")

// Test OuterStruct access
let structNested = OuterStruct.NestedStruct()
print("OuterStruct.NestedStruct accessed")

// Now define OuterClass
class OuterClass {
    var outerProperty: Int = 20
    
    class NestedClass {
        var classProperty: Double = 2.71
    }
}

print("OuterClass defined")

// Test OuterClass access
let classNested = OuterClass.NestedClass()
print("OuterClass.NestedClass accessed")