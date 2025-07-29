// MiniSwift Library - IO and Network APIs

/// Write content to a file
/// - Parameters:
///   - filename: The name of the file to write to
///   - content: The content to write to the file
/// - Returns: True if successful, false otherwise
public func writeFile(_ filename: String, _ content: String) -> Bool {
    // This will be handled by the interpreter's builtin function
    return __builtin_writeFile(filename, content)
}

/// Read content from a file
/// - Parameter filename: The name of the file to read from
/// - Returns: The content of the file as a string
public func readFile(_ filename: String) -> String {
    // This will be handled by the interpreter's builtin function
    return __builtin_readFile(filename)
}

/// Check if a file exists
/// - Parameter filename: The name of the file to check
/// - Returns: True if the file exists, false otherwise
public func fileExists(_ filename: String) -> Bool {
    // This will be handled by the interpreter's builtin function
    return __builtin_fileExists(filename)
}

/// Make an HTTP GET request
/// - Parameter url: The URL to make the request to
/// - Returns: The response content as a string
public func httpGet(_ url: String) -> String {
    // This will be handled by the interpreter's builtin function
    return __builtin_httpGet(url)
}

/// Make an HTTP POST request
/// - Parameters:
///   - url: The URL to make the request to
///   - data: The data to send in the request body
/// - Returns: The response content as a string
public func httpPost(_ url: String, _ data: String) -> String {
    // This will be handled by the interpreter's builtin function
    return __builtin_httpPost(url, data)
}