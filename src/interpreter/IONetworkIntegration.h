#ifndef MINISWIFT_IO_NETWORK_INTEGRATION_H
#define MINISWIFT_IO_NETWORK_INTEGRATION_H

#include "Value.h"
#include "IO/IORuntime.h"
#include "Network/NetworkRuntime.h"
#include "Concurrency/ConcurrencyRuntime.h"
#include "Memory/MemoryManager.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace miniswift {

class Interpreter;
class Environment;

class IONetworkBridge {
public:
    // File operations
    static Value readFile(const std::string& path);
    static Value writeFile(const std::string& path, const Value& content);
    static Value fileExists(const std::string& path);
    static Value deleteFile(const std::string& path);
    static Value copyFile(const std::string& source, const std::string& destination);
    static Value moveFile(const std::string& source, const std::string& destination);
    
    // Async file operations
    static Value readFileAsync(const std::string& path);
    static Value writeFileAsync(const std::string& path, const Value& content);
    
    // Directory operations
    static Value createDirectory(const std::string& path, bool recursive = false);
    static Value removeDirectory(const std::string& path, bool recursive = false);
    static Value listDirectory(const std::string& path);
    static Value directoryExists(const std::string& path);
    
    // HTTP operations
    static Value httpGet(const std::string& url);
    static Value httpPost(const std::string& url, const Value& data);
    static Value httpPut(const std::string& url, const Value& data);
    static Value httpDelete(const std::string& url);
    
    // Async HTTP operations
    static Value httpGetAsync(const std::string& url);
    static Value httpPostAsync(const std::string& url, const Value& data);
    
    // Socket operations
    static Value createTCPSocket();
    static Value connectTCP(const Value& socket, const std::string& host, uint16_t port);
    static Value sendTCP(const Value& socket, const Value& data);
    static Value receiveTCP(const Value& socket, size_t maxSize = 4096);
    static Value closeTCP(const Value& socket);
    
    // UDP operations
    static Value createUDPSocket();
    static Value bindUDP(const Value& socket, const std::string& host, uint16_t port);
    static Value sendToUDP(const Value& socket, const Value& data, const std::string& host, uint16_t port);
    static Value receiveFromUDP(const Value& socket, size_t maxSize = 4096);
    static Value closeUDP(const Value& socket);
    
    // DNS operations
    static Value resolveHostname(const std::string& hostname);
    static Value reverseResolveIP(const std::string& ipAddress);
    
    // Error handling
    static Value createIOError(const std::string& message);
    static Value createNetworkError(const std::string& message);
    static Value createSuccessResult(const Value& data = Value());
    
public:
    // Conversion utilities
    static Value convertIOResultToValue(const IOResult& result);
    static Value convertNetworkResultToValue(const NetworkResult& result);
    static std::vector<uint8_t> valueToByteArray(const Value& value);
    static Value byteArrayToValue(const std::vector<uint8_t>& data);
};

class AsyncOperationWrapper {
public:
    enum class OperationType {
        IO_READ,
        IO_WRITE,
        NETWORK_REQUEST,
        DNS_RESOLVE
    };
    
private:
    OperationType type_;
    std::shared_ptr<AsyncIOOperation> ioOperation_;
    std::shared_ptr<AsyncNetworkOperation> networkOperation_;
    std::function<void(const Value&)> completion_;
    
public:
    AsyncOperationWrapper(std::shared_ptr<AsyncIOOperation> op);
    AsyncOperationWrapper(std::shared_ptr<AsyncNetworkOperation> op);
    
    // Wait for completion
    Value wait();
    
    // Check if ready
    bool isReady() const;
    
    // Cancel operation
    void cancel();
    
    // Set completion callback
    void onCompletion(std::function<void(const Value&)> callback);
};

class HTTPClientWrapper {
private:
    std::unique_ptr<HTTPClient> client_;
    
public:
    HTTPClientWrapper();
    ~HTTPClientWrapper() = default;
    
    // Configuration
    void setTimeout(int milliseconds);
    void setHeader(const std::string& name, const std::string& value);
    void setKeepAlive(bool keepAlive);
    
    // Synchronous requests
    Value get(const std::string& url);
    Value post(const std::string& url, const Value& data);
    Value put(const std::string& url, const Value& data);
    Value delete_(const std::string& url);
    
    // Asynchronous requests
    Value getAsync(const std::string& url);
    Value postAsync(const std::string& url, const Value& data);
    Value putAsync(const std::string& url, const Value& data);
    Value deleteAsync(const std::string& url);
};

class HTTPServerWrapper {
public:
    using RequestHandler = std::function<Value(const Value& request)>;
    
private:
    std::unique_ptr<HTTPServer> server_;
    std::unordered_map<std::string, RequestHandler> routes_;
    RequestHandler defaultHandler_;
    
public:
    HTTPServerWrapper();
    ~HTTPServerWrapper() = default;
    
    // Configuration
    void setMaxConnections(size_t maxConn);
    void setRequestTimeout(int milliseconds);
    
    // Route management
    void addRoute(const std::string& path, RequestHandler handler);
    void setDefaultHandler(RequestHandler handler);
    
    // Server control
    Value start(const std::string& host, uint16_t port);
    void stop();
    bool isRunning() const;
    
private:
    // Conversion helpers
    Value convertHTTPRequestToValue(const HTTPRequest& request);
    HTTPResponse convertValueToHTTPResponse(const Value& response);
};

class FileSystemWatcher {
public:
    enum class EventType {
        Created,
        Modified,
        Deleted,
        Moved
    };
    
    struct FileEvent {
        EventType type;
        std::string path;
        std::string oldPath;
        std::chrono::system_clock::time_point timestamp;
    };
    
    using EventHandler = std::function<void(const FileEvent&)>;
    
private:
    std::string watchPath_;
    EventHandler handler_;
    std::atomic<bool> running_;
    std::thread watchThread_;
    
public:
    FileSystemWatcher(const std::string& path, EventHandler handler);
    ~FileSystemWatcher();
    
    void start();
    void stop();
    bool isRunning() const;
    
private:
    void watchLoop();
};

class IONetworkRegistry {
public:
    // Registration functions
    static void registerIOFunctions(Environment& globalEnv);
    static void registerNetworkFunctions(Environment& globalEnv);
    static void registerAsyncFunctions(Environment& globalEnv);
    static void registerHTTPFunctions(Environment& globalEnv);
    static void registerSocketFunctions(Environment& globalEnv);
    static void registerFileSystemFunctions(Environment& globalEnv);
    
    // Lifecycle
    static void initialize();
    static void shutdown();
    
private:
    // Internal registration helpers
    static void registerFileOperations(Environment& env);
    static void registerDirectoryOperations(Environment& env);
    static void registerNetworkOperations(Environment& env);
    static void registerAsyncOperations(Environment& env);
};

template<typename T>
struct SwiftResult {
    bool isSuccess;
    T value;
    std::string errorMessage;
    
    SwiftResult() : isSuccess(false) {}
    SwiftResult(bool success, const T& val, const std::string& error = "")
        : isSuccess(success), value(val), errorMessage(error) {}
    
    static SwiftResult success(const T& val) {
        return SwiftResult(true, val);
    }
    
    static SwiftResult failure(const std::string& error) {
        SwiftResult result;
        result.isSuccess = false;
        result.errorMessage = error;
        return result;
    }
    
    bool operator==(const SwiftResult& other) const {
        return isSuccess == other.isSuccess && 
               value == other.value && 
               errorMessage == other.errorMessage;
    }
};

using IOResult_Swift = SwiftResult<Value>;
using NetworkResult_Swift = SwiftResult<Value>;
using StringResult = SwiftResult<std::string>;
using BoolResult = SwiftResult<bool>;

void initializeIONetworkSupport();
void shutdownIONetworkSupport();

namespace IONetworkErrors {
    constexpr const char* FILE_NOT_FOUND = "FileNotFound";
    constexpr const char* PERMISSION_DENIED = "PermissionDenied";
    constexpr const char* NETWORK_UNREACHABLE = "NetworkUnreachable";
    constexpr const char* CONNECTION_REFUSED = "ConnectionRefused";
    constexpr const char* TIMEOUT = "Timeout";
    constexpr const char* INVALID_URL = "InvalidURL";
    constexpr const char* DNS_RESOLUTION_FAILED = "DNSResolutionFailed";
    // constexpr const char* SOCKET_ERROR = "SocketError";
    constexpr const char* HTTP_ERROR = "HTTPError";
    constexpr const char* IO_ERROR = "IOError";
}

}

#endif