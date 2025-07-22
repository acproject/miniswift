class OuterClass {
    class NestedClass {
        var value: Int = 42
    }
}

print("OuterClass defined")

// Try to access the nested type
let nested = OuterClass.NestedClass()
print("NestedClass instantiated")
print(nested.value)