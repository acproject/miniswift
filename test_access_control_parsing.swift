// Comprehensive test file for access control parsing

// Test public class with various access levels
public class PublicClass {
    public var publicVar: Int = 10
    private var privateVar: String = "private"
    internal var internalVar: Bool = true
    fileprivate var fileprivateVar: Double = 2.5
    
    public func publicMethod() {
        print("public method")
    }
    
    private func privateMethod() {
        print("private method")
    }
}

// Test private struct
private struct PrivateStruct {
    fileprivate var fileprivateVar: Bool = true
    package var packageVar: Double = 3.14
    internal var internalVar: String = "internal"
}

// Test internal class
internal class InternalClass {
    open var openVar: Int = 1
    public var publicVar: String = "public"
    internal var internalVar: Bool = false
    private var privateVar: [Int] = [1, 2, 3]
}

// Test open function
open func openFunction() {
    print("open function")
}

// Test fileprivate function
fileprivate func fileprivateFunction() {
    print("fileprivate function")
}

// Test internal function
internal func internalFunction() {
    print("internal function")
}

// Test private function
private func privateFunction() {
    print("private function")
}

// Test package variable
package var packageVariable: Int = 42

// Test public variable
public var publicVariable: String = "public"

// Test private variable
private var privateVariable: Bool = false