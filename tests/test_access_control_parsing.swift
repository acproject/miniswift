// Test access control parsing

// Test public class
public class PublicClass {
    public var publicVar = "public"
    private var privateVar = "private"
    internal func internalMethod() {
        print("internal method")
    }
}

// Test private struct
private struct PrivateStruct {
    fileprivate var fileprivateVar = 42
    public func publicMethod() {
        print("public method in private struct")
    }
}

// Test function with access control
open func openFunction() {
    print("open function")
}

package var packageVariable = true

// Test private(set) syntax
public var readOnlyVar: String = "read-only" {
    private(set)
}

fileprivate func fileprivateFunction() {
    print("fileprivate function")
}