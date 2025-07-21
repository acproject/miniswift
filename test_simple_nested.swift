// Simple test for nested types

struct OuterStruct {
    var outerProperty: Int = 10
    
    struct NestedStruct {
        var nestedProperty: String = "nested"
    }
}

print("Testing nested type registration...")

// Try to access the nested type
let nested = OuterStruct.NestedStruct()
print("Nested type access successful")