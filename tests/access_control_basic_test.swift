// Basic Access Control Test
// This file tests the lexical analysis of access control keywords

// Test all access levels
public class PublicClass {
    public var publicProperty = "public"
    internal var internalProperty = "internal"
    fileprivate var fileprivateProperty = "fileprivate"
    private var privateProperty = "private"
    
    public func publicMethod() {
        print("public method")
    }
    
    internal func internalMethod() {
        print("internal method")
    }
    
    fileprivate func fileprivateMethod() {
        print("fileprivate method")
    }
    
    private func privateMethod() {
        print("private method")
    }
}

internal struct InternalStruct {
    var defaultProperty = "default internal"
    private var privateProperty = "private"
    
    func defaultMethod() {
        print("default internal method")
    }
}

fileprivate struct FilePrivateStruct {
    var property = "fileprivate struct property"
}

private struct PrivateStruct {
    var property = "private struct property"
}

// Test getter/setter access control
struct TrackedString {
    private(set) var numberOfEdits = 0
    var value: String = "" {
        didSet {
            numberOfEdits += 1
        }
    }
}

// Test open access level (for inheritance)
open class OpenBaseClass {
    open func openMethod() {
        print("open method")
    }
    
    public func publicMethod() {
        print("public method")
    }
}

public class DerivedClass: OpenBaseClass {
    override public func openMethod() {
        print("overridden open method")
    }
}

// Test package access level
package struct PackageStruct {
    package var packageProperty = "package"
    
    package func packageMethod() {
        print("package method")
    }
}