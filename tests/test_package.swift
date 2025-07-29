// Test package access level
package struct PackageStruct {
    package var packageProperty = "package"
    
    package func packageMethod() {
        print("package method")
    }
}