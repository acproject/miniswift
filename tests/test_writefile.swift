// Test writeFile builtin function
import MiniSwift

// Test basic file writing
writeFile("test_output.txt", "Hello, World!")
print("File written successfully")

// Test with different content
writeFile("test_output2.txt", "This is a test file\nwith multiple lines")
print("Second file written successfully")

// Test with variables
let filename = "test_output3.txt"
let content = "Content from variables"
writeFile(filename, content)
print("Third file written successfully")