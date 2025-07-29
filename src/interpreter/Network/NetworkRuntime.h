#ifndef MINISWIFT_NETWORK_RUNTIME_H
#define MINISWIFT_NETWORK_RUNTIME_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <atomic>
#include <unordered_map>

// 平台特定的网络头文件
#ifdef _WIN32
    // Windows网络支持 - 仅在Windows平台编译时包含
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
    #define SHUT_RD SD_RECEIVE
    #define SHUT_WR SD_SEND
    #define SHUT_RDWR SD_BOTH
    typedef int socklen_t;
    typedef SOCKET socket_t;
#else
    // Unix/Linux/macOS网络支持
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <poll.h>
    typedef int socket_t;
#endif
#include "../Value.h"
#include "../Concurrency/ConcurrencyRuntime.h"
#include "../Memory/MemoryManager.h"
#include "../IO/IORuntime.h"

namespace miniswift {

// Forward declarations
class Socket;
class TCPSocket;
class UDPSocket;
class HTTPClient;
class HTTPServer;
class NetworkScheduler;
class SSLContext;

// 网络协议类型
enum class NetworkProtocol {
    TCP,
    UDP,
    HTTP,
    HTTPS,
    WebSocket
};

// 网络操作类型
enum class NetworkOperationType {
    Connect,
    Disconnect,
    Send,
    Receive,
    Listen,
    Accept,
    Resolve
};

// 网络操作结果
struct NetworkResult {
    bool success;
    std::string errorMessage;
    size_t bytesTransferred;
    std::vector<uint8_t> data;
    std::chrono::milliseconds duration;
    std::string remoteAddress;
    uint16_t remotePort;
    
    NetworkResult() : success(false), bytesTransferred(0), remotePort(0) {}
    NetworkResult(bool s, const std::string& err = "") : success(s), errorMessage(err), bytesTransferred(0), remotePort(0) {}
};

// 网络地址信息
struct NetworkAddress {
    std::string host;
    uint16_t port;
    NetworkProtocol protocol;
    
    NetworkAddress() : port(0), protocol(NetworkProtocol::TCP) {}
    NetworkAddress(const std::string& h, uint16_t p, NetworkProtocol proto = NetworkProtocol::TCP)
        : host(h), port(p), protocol(proto) {}
    
    std::string toString() const {
        return host + ":" + std::to_string(port);
    }
};

// 异步网络操作句柄
class AsyncNetworkOperation {
public:
    using CompletionHandler = std::function<void(const NetworkResult&)>;
    
private:
    std::future<NetworkResult> future_;
    CompletionHandler completion_;
    std::atomic<bool> cancelled_;
    NetworkOperationType type_;
    NetworkAddress address_;
    
public:
    AsyncNetworkOperation(std::future<NetworkResult> future, NetworkOperationType type, const NetworkAddress& addr);
    
    // 等待操作完成
    NetworkResult wait();
    
    // 检查操作是否完成
    bool isReady() const;
    
    // 取消操作
    void cancel();
    
    // 设置完成回调
    void onCompletion(CompletionHandler handler);
    
    // 获取操作信息
    NetworkOperationType getType() const { return type_; }
    const NetworkAddress& getAddress() const { return address_; }
    bool isCancelled() const { return cancelled_.load(); }
};

// 基础Socket类
class Socket {
protected:
    socket_t sockfd_;
    NetworkAddress localAddress_;
    NetworkAddress remoteAddress_;
    bool connected_;
    bool listening_;
    mutable std::mutex mutex_;
    
public:
    Socket(NetworkProtocol protocol);
    virtual ~Socket();
    
    // 禁止拷贝，允许移动
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    
    // 基础操作
    virtual bool bind(const NetworkAddress& address);
    virtual bool connect(const NetworkAddress& address);
    virtual void close();
    
    // 数据传输
    virtual NetworkResult send(const std::vector<uint8_t>& data);
    virtual NetworkResult receive(size_t maxSize = 4096);
    
    // 异步操作
    virtual std::shared_ptr<AsyncNetworkOperation> sendAsync(const std::vector<uint8_t>& data);
    virtual std::shared_ptr<AsyncNetworkOperation> receiveAsync(size_t maxSize = 4096);
    
    // 状态查询
    bool isConnected() const { return connected_; }
    bool isListening() const { return listening_; }
    socket_t getSocketFd() const { return sockfd_; }
    const NetworkAddress& getLocalAddress() const { return localAddress_; }
    const NetworkAddress& getRemoteAddress() const { return remoteAddress_; }
    
    // 套接字选项
    bool setNonBlocking(bool nonBlocking);
    bool setReuseAddress(bool reuse);
    bool setKeepAlive(bool keepAlive);
    bool setTimeout(std::chrono::milliseconds timeout);
    
protected:
    virtual bool createSocket(NetworkProtocol protocol);
    NetworkResult performSend(const uint8_t* data, size_t size);
    NetworkResult performReceive(uint8_t* buffer, size_t maxSize);
};

// TCP Socket类
class TCPSocket : public Socket {
private:
    std::queue<std::shared_ptr<TCPSocket>> pendingConnections_;
    size_t backlogSize_;
    
public:
    TCPSocket();
    explicit TCPSocket(socket_t existingSocket);
    
    // TCP特有操作
    bool listen(size_t backlog = 10);
    std::shared_ptr<TCPSocket> accept();
    std::shared_ptr<AsyncNetworkOperation> acceptAsync();
    
    // 连接管理
    bool shutdown(int how = SHUT_RDWR);
    
    // 获取连接信息
    bool getPeerAddress(NetworkAddress& address) const;
    bool getLocalAddress(NetworkAddress& address) const;
};

// UDP Socket类
class UDPSocket : public Socket {
public:
    UDPSocket();
    
    // UDP特有操作
    NetworkResult sendTo(const std::vector<uint8_t>& data, const NetworkAddress& address);
    NetworkResult receiveFrom(size_t maxSize, NetworkAddress& fromAddress);
    
    // 异步UDP操作
    std::shared_ptr<AsyncNetworkOperation> sendToAsync(const std::vector<uint8_t>& data, const NetworkAddress& address);
    std::shared_ptr<AsyncNetworkOperation> receiveFromAsync(size_t maxSize);
    
    // 广播和多播
    bool enableBroadcast(bool enable);
    bool joinMulticastGroup(const std::string& groupAddress);
    bool leaveMulticastGroup(const std::string& groupAddress);
};

// HTTP请求类型
enum class HTTPMethod {
    GET,
    POST,
    PUT,
    DELETE,
    HEAD,
    OPTIONS,
    PATCH
};

// HTTP头部
using HTTPHeaders = std::unordered_map<std::string, std::string>;

// HTTP请求
struct HTTPRequest {
    HTTPMethod method;
    std::string url;
    std::string path;
    HTTPHeaders headers;
    std::vector<uint8_t> body;
    std::string version; // HTTP/1.1, HTTP/2.0等
    
    HTTPRequest() : method(HTTPMethod::GET), version("HTTP/1.1") {}
};

// HTTP响应
struct HTTPResponse {
    int statusCode;
    std::string statusMessage;
    HTTPHeaders headers;
    std::vector<uint8_t> body;
    std::string version;
    
    HTTPResponse() : statusCode(0), version("HTTP/1.1") {}
    
    std::string getBodyAsString() const {
        return std::string(body.begin(), body.end());
    }
};

// HTTP客户端
class HTTPClient {
private:
    std::unique_ptr<TCPSocket> socket_;
    std::chrono::milliseconds timeout_;
    HTTPHeaders defaultHeaders_;
    bool keepAlive_;
    
public:
    HTTPClient();
    ~HTTPClient();
    
    // 配置
    void setTimeout(std::chrono::milliseconds timeout) { timeout_ = timeout; }
    void setDefaultHeader(const std::string& name, const std::string& value);
    void setKeepAlive(bool keepAlive) { keepAlive_ = keepAlive; }
    
    // 同步HTTP请求
    NetworkResult request(const HTTPRequest& request, HTTPResponse& response);
    NetworkResult get(const std::string& url, HTTPResponse& response);
    NetworkResult post(const std::string& url, const std::vector<uint8_t>& data, HTTPResponse& response);
    NetworkResult post(const std::string& url, const std::string& data, HTTPResponse& response);
    
    // 异步HTTP请求
    std::shared_ptr<AsyncNetworkOperation> requestAsync(const HTTPRequest& request);
    std::shared_ptr<AsyncNetworkOperation> getAsync(const std::string& url);
    std::shared_ptr<AsyncNetworkOperation> postAsync(const std::string& url, const std::vector<uint8_t>& data);
    
private:
    bool parseURL(const std::string& url, std::string& host, uint16_t& port, std::string& path);
    std::string buildRequestString(const HTTPRequest& request);
    bool parseResponse(const std::string& responseData, HTTPResponse& response);
    bool connectToHost(const std::string& host, uint16_t port);
};

// HTTP服务器请求处理器
using HTTPRequestHandler = std::function<void(const HTTPRequest&, HTTPResponse&)>;

// HTTP服务器
class HTTPServer {
private:
    std::unique_ptr<TCPSocket> serverSocket_;
    std::unordered_map<std::string, HTTPRequestHandler> routes_;
    HTTPRequestHandler defaultHandler_;
    std::atomic<bool> running_;
    std::vector<std::thread> workerThreads_;
    size_t maxConnections_;
    std::chrono::milliseconds requestTimeout_;
    
public:
    HTTPServer();
    ~HTTPServer();
    
    // 服务器配置
    void setMaxConnections(size_t maxConn) { maxConnections_ = maxConn; }
    void setRequestTimeout(std::chrono::milliseconds timeout) { requestTimeout_ = timeout; }
    
    // 路由注册
    void addRoute(const std::string& path, HTTPRequestHandler handler);
    void setDefaultHandler(HTTPRequestHandler handler);
    
    // 服务器控制
    bool start(const NetworkAddress& address);
    void stop();
    bool isRunning() const { return running_.load(); }
    
private:
    void acceptLoop();
    void handleClient(std::shared_ptr<TCPSocket> clientSocket);
    bool parseRequest(const std::string& requestData, HTTPRequest& request);
    std::string buildResponseString(const HTTPResponse& response);
    HTTPRequestHandler findHandler(const std::string& path);
};

// 网络调度器
class NetworkScheduler {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> taskQueue_;
    mutable std::mutex queueMutex_;
    std::condition_variable queueCondition_;
    std::atomic<bool> running_;
    
    // 事件循环相关
    std::vector<pollfd> pollFds_;
    std::unordered_map<socket_t, std::function<void()>> fdHandlers_;
    std::mutex pollMutex_;
    std::thread eventLoopThread_;
    
public:
    NetworkScheduler(size_t numThreads = std::thread::hardware_concurrency());
    ~NetworkScheduler();
    
    // 任务调度
    template<typename Func>
    auto schedule(Func&& func) -> std::future<decltype(func())>;
    
    // 事件循环
    void registerSocket(socket_t sockfd, std::function<void()> handler);
    void unregisterSocket(socket_t sockfd);
    
    // 启动和停止
    void start();
    void stop();
    
    bool isRunning() const { return running_.load(); }
    size_t queueSize() const;
    
private:
    void workerLoop();
    void eventLoop();
};

// DNS解析器
class DNSResolver {
public:
    struct DNSResult {
        bool success;
        std::string errorMessage;
        std::vector<std::string> addresses;
        std::chrono::milliseconds duration;
        
        DNSResult() : success(false) {}
    };
    
    // 同步解析
    static DNSResult resolve(const std::string& hostname);
    static DNSResult reverseResolve(const std::string& ipAddress);
    
    // 异步解析
    static std::shared_ptr<AsyncNetworkOperation> resolveAsync(const std::string& hostname);
    static std::shared_ptr<AsyncNetworkOperation> reverseResolveAsync(const std::string& ipAddress);
    
private:
    static DNSResult performResolve(const std::string& hostname, bool reverse = false);
};

// 网络运行时主类
class NetworkRuntime {
private:
    static std::unique_ptr<NetworkRuntime> instance_;
    static std::mutex instanceMutex_;
    
    std::unique_ptr<NetworkScheduler> scheduler_;
    std::unordered_map<socket_t, std::shared_ptr<Socket>> activeSockets_;
    mutable std::mutex socketsMutex_;
    
    NetworkRuntime();
    
public:
    ~NetworkRuntime();
    
    // 单例访问
    static NetworkRuntime& getInstance();
    static void initialize();
    static void shutdown();
    
    // Socket工厂方法
    std::shared_ptr<TCPSocket> createTCPSocket();
    std::shared_ptr<UDPSocket> createUDPSocket();
    std::unique_ptr<HTTPClient> createHTTPClient();
    std::unique_ptr<HTTPServer> createHTTPServer();
    
    // Socket管理
    void registerSocket(std::shared_ptr<Socket> socket);
    void unregisterSocket(socket_t sockfd);
    std::shared_ptr<Socket> getSocket(socket_t sockfd);
    
    // 便捷方法
    NetworkResult httpGet(const std::string& url);
    NetworkResult httpPost(const std::string& url, const std::string& data);
    std::shared_ptr<AsyncNetworkOperation> httpGetAsync(const std::string& url);
    std::shared_ptr<AsyncNetworkOperation> httpPostAsync(const std::string& url, const std::string& data);
    
    // DNS解析
    DNSResolver::DNSResult resolveHostname(const std::string& hostname);
    std::shared_ptr<AsyncNetworkOperation> resolveHostnameAsync(const std::string& hostname);
    
    // 获取调度器
    NetworkScheduler& getScheduler() { return *scheduler_; }
    
    // 网络统计
    size_t getActiveSocketCount() const;
    std::vector<NetworkAddress> getActiveConnections() const;
};

// 模板实现
template<typename Func>
auto NetworkScheduler::schedule(Func&& func) -> std::future<decltype(func())> {
    using ReturnType = decltype(func());
    
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::forward<Func>(func)
    );
    
    std::future<ReturnType> result = task->get_future();
    
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        if (!running_.load()) {
            throw std::runtime_error("NetworkScheduler is not running");
        }
        
        taskQueue_.emplace([task]() {
            (*task)();
        });
    }
    
    queueCondition_.notify_one();
    return result;
}

} // namespace miniswift

#endif // MINISWIFT_NETWORK_RUNTIME_H