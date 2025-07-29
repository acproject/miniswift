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

// Forward declarations
class Interpreter;
class Environment;

// IO和Network功能的Swift风格包装器
class IONetworkBridge {
public:
    // 文件操作的Swift风格接口
    static Value readFile(const std::string& path);
    static Value writeFile(const std::string& path, const Value& content);
    static Value fileExists(const std::string& path);
    static Value deleteFile(const std::string& path);
    static Value copyFile(const std::string& source, const std::string& destination);
    static Value moveFile(const std::string& source, const std::string& destination);
    
    // 异步文件操作
    static Value readFileAsync(const std::string& path);
    static Value writeFileAsync(const std::string& path, const Value& content);
    
    // 目录操作
    static Value createDirectory(const std::string& path, bool recursive = false);
    static Value removeDirectory(const std::string& path, bool recursive = false);
    static Value listDirectory(const std::string& path);
    static Value directoryExists(const std::string& path);
    
    // 网络操作的Swift风格接口
    static Value httpGet(const std::string& url);
    static Value httpPost(const std::string& url, const Value& data);
    static Value httpPut(const std::string& url, const Value& data);
    static Value httpDelete(const std::string& url);
    
    // 异步网络操作
    static Value httpGetAsync(const std::string& url);
    static Value httpPostAsync(const std::string& url, const Value& data);
    
    // TCP Socket操作
    static Value createTCPSocket();
    static Value connectTCP(const Value& socket, const std::string& host, uint16_t port);
    static Value sendTCP(const Value& socket, const Value& data);
    static Value receiveTCP(const Value& socket, size_t maxSize = 4096);
    static Value closeTCP(const Value& socket);
    
    // UDP Socket操作
    static Value createUDPSocket();
    static Value bindUDP(const Value& socket, const std::string& host, uint16_t port);
    static Value sendToUDP(const Value& socket, const Value& data, const std::string& host, uint16_t port);
    static Value receiveFromUDP(const Value& socket, size_t maxSize = 4096);
    static Value closeUDP(const Value& socket);
    
    // DNS解析
    static Value resolveHostname(const std::string& hostname);
    static Value reverseResolveIP(const std::string& ipAddress);
    
    // 错误处理辅助函数
    static Value createIOError(const std::string& message);
    static Value createNetworkError(const std::string& message);
    static Value createSuccessResult(const Value& data = Value());
    
public:
    // 辅助方法
    static Value convertIOResultToValue(const IOResult& result);
    static Value convertNetworkResultToValue(const NetworkResult& result);
    static std::vector<uint8_t> valueToByteArray(const Value& value);
    static Value byteArrayToValue(const std::vector<uint8_t>& data);
};

// Swift风格的异步操作包装器
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
    
    // 等待操作完成
    Value wait();
    
    // 检查是否完成
    bool isReady() const;
    
    // 取消操作
    void cancel();
    
    // 设置完成回调
    void onCompletion(std::function<void(const Value&)> callback);
    
    OperationType getType() const { return type_; }
};

// HTTP客户端的Swift风格包装器
class HTTPClientWrapper {
private:
    std::unique_ptr<HTTPClient> client_;
    
public:
    HTTPClientWrapper();
    ~HTTPClientWrapper() = default;
    
    // 配置方法
    void setTimeout(int milliseconds);
    void setHeader(const std::string& name, const std::string& value);
    void setKeepAlive(bool keepAlive);
    
    // HTTP请求方法
    Value get(const std::string& url);
    Value post(const std::string& url, const Value& data);
    Value put(const std::string& url, const Value& data);
    Value delete_(const std::string& url);
    
    // 异步HTTP请求方法
    Value getAsync(const std::string& url);
    Value postAsync(const std::string& url, const Value& data);
    Value putAsync(const std::string& url, const Value& data);
    Value deleteAsync(const std::string& url);
};

// HTTP服务器的Swift风格包装器
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
    
    // 服务器配置
    void setMaxConnections(size_t maxConn);
    void setRequestTimeout(int milliseconds);
    
    // 路由注册
    void addRoute(const std::string& path, RequestHandler handler);
    void setDefaultHandler(RequestHandler handler);
    
    // 服务器控制
    Value start(const std::string& host, uint16_t port);
    void stop();
    bool isRunning() const;
    
private:
    // 内部转换函数
    Value convertHTTPRequestToValue(const HTTPRequest& request);
    HTTPResponse convertValueToHTTPResponse(const Value& response);
};

// 文件系统监视器
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
        std::string oldPath; // 用于移动事件
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
    bool isRunning() const { return running_.load(); }
    
private:
    void watchLoop();
};

// IO和Network功能注册器
class IONetworkRegistry {
public:
    // 向解释器注册IO和Network功能
    static void registerIOFunctions(Environment& globalEnv);
    static void registerNetworkFunctions(Environment& globalEnv);
    static void registerAsyncFunctions(Environment& globalEnv);
    static void registerHTTPFunctions(Environment& globalEnv);
    static void registerSocketFunctions(Environment& globalEnv);
    static void registerFileSystemFunctions(Environment& globalEnv);
    
    // 初始化和清理
    static void initialize();
    static void shutdown();
    
private:
    // 内部注册函数
    static void registerFileOperations(Environment& env);
    static void registerDirectoryOperations(Environment& env);
    static void registerNetworkOperations(Environment& env);
    static void registerAsyncOperations(Environment& env);
};

// Swift风格的Result类型
template<typename T>
struct SwiftResult {
    bool isSuccess;
    T value;
    std::string errorMessage;
    
    SwiftResult() : isSuccess(false) {}
    SwiftResult(const T& val) : isSuccess(true), value(val) {}
    SwiftResult(const std::string& error) : isSuccess(false), errorMessage(error) {}
    
    static SwiftResult success(const T& val) {
        return SwiftResult(val);
    }
    
    static SwiftResult failure(const std::string& error) {
        SwiftResult result;
        result.isSuccess = false;
        result.errorMessage = error;
        return result;
    }
    
    bool operator==(const SwiftResult& other) const {
        if (isSuccess != other.isSuccess) return false;
        if (isSuccess) return value == other.value;
        return errorMessage == other.errorMessage;
    }
};

// 便捷的类型别名
using IOResult_Swift = SwiftResult<Value>;
using NetworkResult_Swift = SwiftResult<Value>;
using StringResult = SwiftResult<std::string>;
using BoolResult = SwiftResult<bool>;

// 全局初始化函数
void initializeIONetworkSupport();
void shutdownIONetworkSupport();

// 错误代码定义
namespace IONetworkErrors {
    constexpr const char* FILE_NOT_FOUND = "FileNotFound";
    constexpr const char* PERMISSION_DENIED = "PermissionDenied";
    constexpr const char* NETWORK_UNREACHABLE = "NetworkUnreachable";
    constexpr const char* CONNECTION_REFUSED = "ConnectionRefused";
    constexpr const char* TIMEOUT = "Timeout";
    constexpr const char* INVALID_URL = "InvalidURL";
    constexpr const char* DNS_RESOLUTION_FAILED = "DNSResolutionFailed";
    constexpr const char* SOCKET_ERROR = "SocketError";
    constexpr const char* HTTP_ERROR = "HTTPError";
    constexpr const char* IO_ERROR = "IOError";
}

} // namespace miniswift

#endif // MINISWIFT_IO_NETWORK_INTEGRATION_H