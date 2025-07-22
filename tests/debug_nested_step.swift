// Debug nested types step by step

class OuterClass {
    var outerProperty: Int = 20
    
    class NestedClass {
        var classProperty: Double = 2.71
    }
}

print("OuterClass with nested type defined")
let outer = OuterClass()
print("OuterClass instantiated")