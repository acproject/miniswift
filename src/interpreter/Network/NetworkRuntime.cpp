#include "NetworkRuntime.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <regex>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
    // Windows下的poll函数映射
    #define poll WSAPoll
    #define pollfd WSAPOLLFD
    #define POLLIN POLLRDNORM
    // Windows下的ssize_t定义
    typedef int ssize_t;
#else
    #include <poll.h>
#endif

namespace miniswift {

// AsyncNetworkOperation 实现
AsyncNetworkOperation::AsyncNetworkOperation(std::future<NetworkResult> future, NetworkOperationType type, const NetworkAddress& addr)
    : future_(std::move(future)), type_(type), address_(addr), cancelled_(false) {
}

NetworkResult AsyncNetworkOperation::wait() {
    if (cancelled_.load()) {
        NetworkResult result;
        result.success = false;
        result.errorMessage = "Operation was cancelled";
        return result;
    }
    
    auto result = future_.get();
    if (completion_) {
        completion_(result);
    }
    return result;
}

bool AsyncNetworkOperation::isReady() const {
    if (cancelled_.load()) {
        return true;
    }
    return future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

void AsyncNetworkOperation::cancel() {
    cancelled_.store(true);
}

void AsyncNetworkOperation::onCompletion(CompletionHandler handler) {
    completion_ = std::move(handler);
}

// Socket 基类实现
Socket::Socket(NetworkProtocol protocol) 
    : sockfd_(-1), connected_(false), listening_(false) {
    createSocket(protocol);
}

Socket::~Socket() {
    close();
}

Socket::Socket(Socket&& other) noexcept
    : sockfd_(other.sockfd_)
    , localAddress_(std::move(other.localAddress_))
    , remoteAddress_(std::move(other.remoteAddress_))
    , connected_(other.connected_)
    , listening_(other.listening_) {
    other.sockfd_ = -1;
    other.connected_ = false;
    other.listening_ = false;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        
        sockfd_ = other.sockfd_;
        localAddress_ = std::move(other.localAddress_);
        remoteAddress_ = std::move(other.remoteAddress_);
        connected_ = other.connected_;
        listening_ = other.listening_;
        
        other.sockfd_ = -1;
        other.connected_ = false;
        other.listening_ = false;
    }
    return *this;
}

bool Socket::createSocket(NetworkProtocol protocol) {
    int domain = AF_INET;
    int type = (protocol == NetworkProtocol::TCP) ? SOCK_STREAM : SOCK_DGRAM;
    int protocolNum = (protocol == NetworkProtocol::TCP) ? IPPROTO_TCP : IPPROTO_UDP;
    
    sockfd_ = socket(domain, type, protocolNum);
    return sockfd_ != -1;
}

bool Socket::bind(const NetworkAddress& address) {
    if (sockfd_ == -1) {
        return false;
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(address.port);
    
    if (address.host.empty() || address.host == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, address.host.c_str(), &addr.sin_addr) <= 0) {
            return false;
        }
    }
    
    int result = ::bind(sockfd_, (struct sockaddr*)&addr, sizeof(addr));
    if (result == 0) {
        localAddress_ = address;
    }
    
    return result == 0;
}

bool Socket::connect(const NetworkAddress& address) {
    if (sockfd_ == -1) {
        return false;
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(address.port);
    
    if (inet_pton(AF_INET, address.host.c_str(), &addr.sin_addr) <= 0) {
        return false;
    }
    
    int result = ::connect(sockfd_, (struct sockaddr*)&addr, sizeof(addr));
    if (result == 0) {
        connected_ = true;
        remoteAddress_ = address;
    }
    
    return result == 0;
}

void Socket::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (sockfd_ != -1) {
#ifdef _WIN32
        closesocket(sockfd_);
#else
        ::close(sockfd_);
#endif
        sockfd_ = -1;
        connected_ = false;
        listening_ = false;
    }
}

NetworkResult Socket::send(const std::vector<uint8_t>& data) {
    return performSend(data.data(), data.size());
}

NetworkResult Socket::receive(size_t maxSize) {
    std::vector<uint8_t> buffer(maxSize);
    auto result = performReceive(buffer.data(), maxSize);
    
    if (result.success && result.bytesTransferred > 0) {
        buffer.resize(result.bytesTransferred);
        result.data = std::move(buffer);
    }
    
    return result;
}

std::shared_ptr<AsyncNetworkOperation> Socket::sendAsync(const std::vector<uint8_t>& data) {
    auto future = NetworkRuntime::getInstance().getScheduler().schedule([this, data]() {
        return this->send(data);
    });
    
    return std::make_shared<AsyncNetworkOperation>(std::move(future), NetworkOperationType::Send, remoteAddress_);
}

std::shared_ptr<AsyncNetworkOperation> Socket::receiveAsync(size_t maxSize) {
    auto future = NetworkRuntime::getInstance().getScheduler().schedule([this, maxSize]() {
        return this->receive(maxSize);
    });
    
    return std::make_shared<AsyncNetworkOperation>(std::move(future), NetworkOperationType::Receive, remoteAddress_);
}

bool Socket::setNonBlocking(bool nonBlocking) {
    if (sockfd_ == -1) {
        return false;
    }
    
#ifdef _WIN32
    u_long mode = nonBlocking ? 1 : 0;
    return ioctlsocket(sockfd_, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(sockfd_, F_GETFL, 0);
    if (flags == -1) {
        return false;
    }
    
    if (nonBlocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    
    return fcntl(sockfd_, F_SETFL, flags) == 0;
#endif
}

bool Socket::setReuseAddress(bool reuse) {
    if (sockfd_ == -1) {
        return false;
    }
    
    int optval = reuse ? 1 : 0;
    return setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optval), sizeof(optval)) == 0;
}

bool Socket::setKeepAlive(bool keepAlive) {
    if (sockfd_ == -1) {
        return false;
    }
    
    int optval = keepAlive ? 1 : 0;
    return setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&optval), sizeof(optval)) == 0;
}

bool Socket::setTimeout(std::chrono::milliseconds timeout) {
    if (sockfd_ == -1) {
        return false;
    }
    
    struct timeval tv;
    tv.tv_sec = timeout.count() / 1000;
    tv.tv_usec = (timeout.count() % 1000) * 1000;
    
    bool result = true;
    result &= setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv)) == 0;
        result &= setsockopt(sockfd_, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv)) == 0;
    
    return result;
}

NetworkResult Socket::performSend(const uint8_t* data, size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    NetworkResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    if (sockfd_ == -1 || !connected_) {
        result.success = false;
        result.errorMessage = "Socket is not connected";
        return result;
    }
    
    ssize_t bytesSent = ::send(sockfd_, reinterpret_cast<const char*>(data), size, 0);
    
    if (bytesSent >= 0) {
        result.success = true;
        result.bytesTransferred = static_cast<size_t>(bytesSent);
    } else {
        result.success = false;
        result.errorMessage = "Send failed: " + std::string(strerror(errno));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    return result;
}

NetworkResult Socket::performReceive(uint8_t* buffer, size_t maxSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    NetworkResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    if (sockfd_ == -1 || !connected_) {
        result.success = false;
        result.errorMessage = "Socket is not connected";
        return result;
    }
    
    ssize_t bytesReceived = ::recv(sockfd_, reinterpret_cast<char*>(buffer), maxSize, 0);
    
    if (bytesReceived > 0) {
        result.success = true;
        result.bytesTransferred = static_cast<size_t>(bytesReceived);
    } else if (bytesReceived == 0) {
        result.success = false;
        result.errorMessage = "Connection closed by peer";
        connected_ = false;
    } else {
        result.success = false;
        result.errorMessage = "Receive failed: " + std::string(strerror(errno));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    return result;
}

// TCPSocket 实现
TCPSocket::TCPSocket() : Socket(NetworkProtocol::TCP), backlogSize_(10) {
}

TCPSocket::TCPSocket(socket_t existingSocket) : Socket(NetworkProtocol::TCP), backlogSize_(10) {
    sockfd_ = existingSocket;
    connected_ = true;
}

bool TCPSocket::listen(size_t backlog) {
    if (sockfd_ == -1) {
        return false;
    }
    
    backlogSize_ = backlog;
    int result = ::listen(sockfd_, static_cast<int>(backlog));
    
    if (result == 0) {
        listening_ = true;
    }
    
    return result == 0;
}

std::shared_ptr<TCPSocket> TCPSocket::accept() {
    if (sockfd_ == -1 || !listening_) {
        return nullptr;
    }
    
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    int clientSocket = ::accept(sockfd_, (struct sockaddr*)&clientAddr, &clientAddrLen);
    
    if (clientSocket == -1) {
        return nullptr;
    }
    
    auto clientTCPSocket = std::make_shared<TCPSocket>(clientSocket);
    
    // 设置远程地址信息
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    clientTCPSocket->remoteAddress_ = NetworkAddress(clientIP, ntohs(clientAddr.sin_port));
    
    return clientTCPSocket;
}

std::shared_ptr<AsyncNetworkOperation> TCPSocket::acceptAsync() {
    auto future = NetworkRuntime::getInstance().getScheduler().schedule([this]() {
        NetworkResult result;
        auto clientSocket = this->accept();
        
        if (clientSocket) {
            result.success = true;
            result.remoteAddress = clientSocket->getRemoteAddress().host;
            result.remotePort = clientSocket->getRemoteAddress().port;
        } else {
            result.success = false;
            result.errorMessage = "Accept failed";
        }
        
        return result;
    });
    
    return std::make_shared<AsyncNetworkOperation>(std::move(future), NetworkOperationType::Accept, localAddress_);
}

bool TCPSocket::shutdown(int how) {
    if (sockfd_ == -1) {
        return false;
    }
    
    return ::shutdown(sockfd_, how) == 0;
}

bool TCPSocket::getPeerAddress(NetworkAddress& address) const {
    if (sockfd_ == -1) {
        return false;
    }
    
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    
    if (getpeername(sockfd_, (struct sockaddr*)&addr, &addrLen) == 0) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
        address = NetworkAddress(ip, ntohs(addr.sin_port));
        return true;
    }
    
    return false;
}

bool TCPSocket::getLocalAddress(NetworkAddress& address) const {
    if (sockfd_ == -1) {
        return false;
    }
    
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    
    if (getsockname(sockfd_, (struct sockaddr*)&addr, &addrLen) == 0) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
        address = NetworkAddress(ip, ntohs(addr.sin_port));
        return true;
    }
    
    return false;
}

// UDPSocket 实现
UDPSocket::UDPSocket() : Socket(NetworkProtocol::UDP) {
}

NetworkResult UDPSocket::sendTo(const std::vector<uint8_t>& data, const NetworkAddress& address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    NetworkResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    if (sockfd_ == -1) {
        result.success = false;
        result.errorMessage = "Socket is not valid";
        return result;
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(address.port);
    
    if (inet_pton(AF_INET, address.host.c_str(), &addr.sin_addr) <= 0) {
        result.success = false;
        result.errorMessage = "Invalid address";
        return result;
    }
    
    ssize_t bytesSent = sendto(sockfd_, reinterpret_cast<const char*>(data.data()), data.size(), 0,
                               (struct sockaddr*)&addr, sizeof(addr));
    
    if (bytesSent >= 0) {
        result.success = true;
        result.bytesTransferred = static_cast<size_t>(bytesSent);
        result.remoteAddress = address.host;
        result.remotePort = address.port;
    } else {
        result.success = false;
        result.errorMessage = "SendTo failed: " + std::string(strerror(errno));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    return result;
}

NetworkResult UDPSocket::receiveFrom(size_t maxSize, NetworkAddress& fromAddress) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    NetworkResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    if (sockfd_ == -1) {
        result.success = false;
        result.errorMessage = "Socket is not valid";
        return result;
    }
    
    std::vector<uint8_t> buffer(maxSize);
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    
    ssize_t bytesReceived = recvfrom(sockfd_, reinterpret_cast<char*>(buffer.data()), maxSize, 0,
                                    (struct sockaddr*)&addr, &addrLen);
    
    if (bytesReceived > 0) {
        result.success = true;
        result.bytesTransferred = static_cast<size_t>(bytesReceived);
        buffer.resize(result.bytesTransferred);
        result.data = std::move(buffer);
        
        // 设置发送方地址
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
        fromAddress = NetworkAddress(ip, ntohs(addr.sin_port));
        result.remoteAddress = fromAddress.host;
        result.remotePort = fromAddress.port;
    } else {
        result.success = false;
        result.errorMessage = "ReceiveFrom failed: " + std::string(strerror(errno));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    return result;
}

std::shared_ptr<AsyncNetworkOperation> UDPSocket::sendToAsync(const std::vector<uint8_t>& data, const NetworkAddress& address) {
    auto future = NetworkRuntime::getInstance().getScheduler().schedule([this, data, address]() {
        return this->sendTo(data, address);
    });
    
    return std::make_shared<AsyncNetworkOperation>(std::move(future), NetworkOperationType::Send, address);
}

std::shared_ptr<AsyncNetworkOperation> UDPSocket::receiveFromAsync(size_t maxSize) {
    auto future = NetworkRuntime::getInstance().getScheduler().schedule([this, maxSize]() {
        NetworkAddress fromAddr;
        return this->receiveFrom(maxSize, fromAddr);
    });
    
    return std::make_shared<AsyncNetworkOperation>(std::move(future), NetworkOperationType::Receive, NetworkAddress());
}

bool UDPSocket::enableBroadcast(bool enable) {
    if (sockfd_ == -1) {
        return false;
    }
    
    int optval = enable ? 1 : 0;
    return setsockopt(sockfd_, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&optval), sizeof(optval)) == 0;
}

bool UDPSocket::joinMulticastGroup(const std::string& groupAddress) {
    if (sockfd_ == -1) {
        return false;
    }
    
    struct ip_mreq mreq;
    if (inet_pton(AF_INET, groupAddress.c_str(), &mreq.imr_multiaddr) <= 0) {
        return false;
    }
    mreq.imr_interface.s_addr = INADDR_ANY;
    
    return setsockopt(sockfd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(mreq)) == 0;
}

bool UDPSocket::leaveMulticastGroup(const std::string& groupAddress) {
    if (sockfd_ == -1) {
        return false;
    }
    
    struct ip_mreq mreq;
    if (inet_pton(AF_INET, groupAddress.c_str(), &mreq.imr_multiaddr) <= 0) {
        return false;
    }
    mreq.imr_interface.s_addr = INADDR_ANY;
    
    return setsockopt(sockfd_, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(mreq)) == 0;
}

// HTTPClient 实现
HTTPClient::HTTPClient() : timeout_(std::chrono::seconds(30)), keepAlive_(false) {
    defaultHeaders_["User-Agent"] = "MiniSwift-HTTPClient/1.0";
    defaultHeaders_["Accept"] = "*/*";
    defaultHeaders_["Connection"] = "close";
}

HTTPClient::~HTTPClient() {
    if (socket_) {
        socket_->close();
    }
}

void HTTPClient::setDefaultHeader(const std::string& name, const std::string& value) {
    defaultHeaders_[name] = value;
}

NetworkResult HTTPClient::request(const HTTPRequest& request, HTTPResponse& response) {
    NetworkResult result;
    
    std::string host;
    uint16_t port;
    std::string path;
    
    if (!parseURL(request.url, host, port, path)) {
        result.success = false;
        result.errorMessage = "Invalid URL";
        return result;
    }
    
    if (!connectToHost(host, port)) {
        result.success = false;
        result.errorMessage = "Failed to connect to host";
        return result;
    }
    
    // 构建请求字符串
    std::string requestStr = buildRequestString(request);
    std::vector<uint8_t> requestData(requestStr.begin(), requestStr.end());
    
    // 发送请求
    auto sendResult = socket_->send(requestData);
    if (!sendResult.success) {
        result.success = false;
        result.errorMessage = "Failed to send request: " + sendResult.errorMessage;
        return result;
    }
    
    // 接收响应
    std::string responseData;
    while (true) {
        auto recvResult = socket_->receive(4096);
        if (!recvResult.success) {
            if (responseData.empty()) {
                result.success = false;
                result.errorMessage = "Failed to receive response: " + recvResult.errorMessage;
                return result;
            }
            break;
        }
        
        if (recvResult.bytesTransferred == 0) {
            break;
        }
        
        responseData.append(recvResult.data.begin(), recvResult.data.end());
        
        // 检查是否接收完整响应（简单实现）
        if (responseData.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }
    
    // 解析响应
    if (!parseResponse(responseData, response)) {
        result.success = false;
        result.errorMessage = "Failed to parse response";
        return result;
    }
    
    result.success = true;
    result.bytesTransferred = responseData.size();
    
    if (!keepAlive_) {
        socket_->close();
        socket_.reset();
    }
    
    return result;
}

NetworkResult HTTPClient::get(const std::string& url, HTTPResponse& response) {
    HTTPRequest request;
    request.method = HTTPMethod::GET;
    request.url = url;
    return this->request(request, response);
}

NetworkResult HTTPClient::post(const std::string& url, const std::vector<uint8_t>& data, HTTPResponse& response) {
    HTTPRequest request;
    request.method = HTTPMethod::POST;
    request.url = url;
    request.body = data;
    request.headers["Content-Length"] = std::to_string(data.size());
    return this->request(request, response);
}

NetworkResult HTTPClient::post(const std::string& url, const std::string& data, HTTPResponse& response) {
    std::vector<uint8_t> bodyData(data.begin(), data.end());
    return post(url, bodyData, response);
}

std::shared_ptr<AsyncNetworkOperation> HTTPClient::requestAsync(const HTTPRequest& request) {
    auto future = NetworkRuntime::getInstance().getScheduler().schedule([this, request]() {
        HTTPResponse response;
        auto result = this->request(request, response);
        
        // 将响应数据编码到结果中
        if (result.success) {
            std::string responseStr = response.getBodyAsString();
            result.data = std::vector<uint8_t>(responseStr.begin(), responseStr.end());
        }
        
        return result;
    });
    
    std::string host;
    uint16_t port;
    std::string path;
    parseURL(request.url, host, port, path);
    
    return std::make_shared<AsyncNetworkOperation>(std::move(future), NetworkOperationType::Send, NetworkAddress(host, port));
}

std::shared_ptr<AsyncNetworkOperation> HTTPClient::getAsync(const std::string& url) {
    HTTPRequest request;
    request.method = HTTPMethod::GET;
    request.url = url;
    return requestAsync(request);
}

std::shared_ptr<AsyncNetworkOperation> HTTPClient::postAsync(const std::string& url, const std::vector<uint8_t>& data) {
    HTTPRequest request;
    request.method = HTTPMethod::POST;
    request.url = url;
    request.body = data;
    request.headers["Content-Length"] = std::to_string(data.size());
    return requestAsync(request);
}

bool HTTPClient::parseURL(const std::string& url, std::string& host, uint16_t& port, std::string& path) {
    std::regex urlRegex(R"(^https?://([^:/]+)(?::(\d+))?(/.*)?$)");
    std::smatch matches;
    
    if (!std::regex_match(url, matches, urlRegex)) {
        return false;
    }
    
    host = matches[1].str();
    
    if (matches[2].matched) {
        port = static_cast<uint16_t>(std::stoi(matches[2].str()));
    } else {
        port = (url.substr(0, 5) == "https") ? 443 : 80;
    }
    
    path = matches[3].matched ? matches[3].str() : "/";
    
    return true;
}

std::string HTTPClient::buildRequestString(const HTTPRequest& request) {
    std::ostringstream oss;
    
    // 请求行
    std::string methodStr;
    switch (request.method) {
        case HTTPMethod::GET: methodStr = "GET"; break;
        case HTTPMethod::POST: methodStr = "POST"; break;
        case HTTPMethod::PUT: methodStr = "PUT"; break;
        case HTTPMethod::DELETE: methodStr = "DELETE"; break;
        case HTTPMethod::HEAD: methodStr = "HEAD"; break;
        case HTTPMethod::OPTIONS: methodStr = "OPTIONS"; break;
        case HTTPMethod::PATCH: methodStr = "PATCH"; break;
    }
    
    std::string host, path;
    uint16_t port;
    parseURL(request.url, host, port, path);
    
    oss << methodStr << " " << path << " " << request.version << "\r\n";
    
    // 头部
    auto headers = defaultHeaders_;
    for (const auto& header : request.headers) {
        headers[header.first] = header.second;
    }
    
    headers["Host"] = host;
    if (!request.body.empty() && headers.find("Content-Length") == headers.end()) {
        headers["Content-Length"] = std::to_string(request.body.size());
    }
    
    for (const auto& header : headers) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    oss << "\r\n";
    
    // 请求体
    if (!request.body.empty()) {
        oss << std::string(request.body.begin(), request.body.end());
    }
    
    return oss.str();
}

bool HTTPClient::parseResponse(const std::string& responseData, HTTPResponse& response) {
    size_t headerEnd = responseData.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return false;
    }
    
    std::string headerSection = responseData.substr(0, headerEnd);
    std::string bodySection = responseData.substr(headerEnd + 4);
    
    // 解析状态行
    size_t firstLine = headerSection.find("\r\n");
    std::string statusLine = headerSection.substr(0, firstLine);
    
    std::istringstream statusStream(statusLine);
    std::string version, statusCodeStr, statusMessage;
    statusStream >> version >> statusCodeStr;
    std::getline(statusStream, statusMessage);
    
    response.version = version;
    response.statusCode = std::stoi(statusCodeStr);
    response.statusMessage = statusMessage.substr(1); // 移除前导空格
    
    // 解析头部
    std::istringstream headerStream(headerSection.substr(firstLine + 2));
    std::string line;
    while (std::getline(headerStream, line) && !line.empty()) {
        if (line.back() == '\r') {
            line.pop_back();
        }
        
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string name = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // 移除前导和尾随空格
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            response.headers[name] = value;
        }
    }
    
    // 设置响应体
    response.body = std::vector<uint8_t>(bodySection.begin(), bodySection.end());
    
    return true;
}

bool HTTPClient::connectToHost(const std::string& host, uint16_t port) {
    if (!socket_ || !socket_->isConnected()) {
        // 首先进行DNS解析
        DNSResolver resolver;
        auto dnsResult = resolver.resolve(host);
        
        if (!dnsResult.success || dnsResult.addresses.empty()) {
            return false;
        }
        
        // 创建新的TCP socket
        socket_ = std::make_unique<TCPSocket>();
        
        // 检查socket是否创建成功
        if (!socket_ || socket_->getSocketFd() == -1) {
            return false;
        }
        
        socket_->setTimeout(timeout_);
        
        // 使用解析得到的第一个IP地址
        NetworkAddress address(dnsResult.addresses[0], port);
        return socket_->connect(address);
    }
    
    return true;
}

// NetworkScheduler 实现
NetworkScheduler::NetworkScheduler(size_t numThreads) : running_(false) {
    workers_.reserve(numThreads);
}

NetworkScheduler::~NetworkScheduler() {
    stop();
}

void NetworkScheduler::start() {
    if (running_.load()) {
        return;
    }
    
    running_.store(true);
    
    // 启动工作线程
    size_t numThreads = workers_.capacity();
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back(&NetworkScheduler::workerLoop, this);
    }
    
    // 启动事件循环线程
    eventLoopThread_ = std::thread(&NetworkScheduler::eventLoop, this);
}

void NetworkScheduler::stop() {
    if (!running_.load()) {
        return;
    }
    
    running_.store(false);
    queueCondition_.notify_all();
    
    // 等待工作线程结束
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers_.clear();
    
    // 等待事件循环线程结束
    if (eventLoopThread_.joinable()) {
        eventLoopThread_.join();
    }
}

void NetworkScheduler::registerSocket(socket_t sockfd, std::function<void()> handler) {
    std::lock_guard<std::mutex> lock(pollMutex_);
    
    pollfd pfd;
    pfd.fd = static_cast<decltype(pfd.fd)>(sockfd);
    pfd.events = POLLIN;
    pfd.revents = 0;
    
    pollFds_.push_back(pfd);
    fdHandlers_[sockfd] = std::move(handler);
}

void NetworkScheduler::unregisterSocket(socket_t sockfd) {
    std::lock_guard<std::mutex> lock(pollMutex_);
    
    auto it = std::find_if(pollFds_.begin(), pollFds_.end(),
                          [sockfd](const pollfd& pfd) { return pfd.fd == static_cast<decltype(pfd.fd)>(sockfd); });
    
    if (it != pollFds_.end()) {
        pollFds_.erase(it);
    }
    
    fdHandlers_.erase(sockfd);
}

size_t NetworkScheduler::queueSize() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return taskQueue_.size();
}

void NetworkScheduler::workerLoop() {
    while (running_.load()) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCondition_.wait(lock, [this] {
                return !taskQueue_.empty() || !running_.load();
            });
            
            if (!running_.load()) {
                break;
            }
            
            if (!taskQueue_.empty()) {
                task = std::move(taskQueue_.front());
                taskQueue_.pop();
            }
        }
        
        if (task) {
            try {
                task();
            } catch (const std::exception& e) {
                std::cerr << "NetworkScheduler task error: " << e.what() << std::endl;
            }
        }
    }
}

void NetworkScheduler::eventLoop() {
    while (running_.load()) {
        std::vector<pollfd> fds;
        std::unordered_map<socket_t, std::function<void()>> handlers;
        
        {
            std::lock_guard<std::mutex> lock(pollMutex_);
            fds = pollFds_;
            handlers = fdHandlers_;
        }
        
        if (fds.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        int result = poll(fds.data(), fds.size(), 100); // 100ms timeout
        
        if (result > 0) {
            for (const auto& pfd : fds) {
                if (pfd.revents & POLLIN) {
                    auto it = handlers.find(pfd.fd);
                    if (it != handlers.end()) {
                        try {
                            it->second();
                        } catch (const std::exception& e) {
                            std::cerr << "Event handler error: " << e.what() << std::endl;
                        }
                    }
                }
            }
        }
    }
}

// DNSResolver 实现
DNSResolver::DNSResult DNSResolver::resolve(const std::string& hostname) {
    return performResolve(hostname, false);
}

DNSResolver::DNSResult DNSResolver::reverseResolve(const std::string& ipAddress) {
    return performResolve(ipAddress, true);
}

std::shared_ptr<AsyncNetworkOperation> DNSResolver::resolveAsync(const std::string& hostname) {
    auto future = NetworkRuntime::getInstance().getScheduler().schedule([hostname]() {
        auto dnsResult = resolve(hostname);
        
        NetworkResult result;
        result.success = dnsResult.success;
        result.errorMessage = dnsResult.errorMessage;
        result.duration = dnsResult.duration;
        
        if (dnsResult.success && !dnsResult.addresses.empty()) {
            std::string addressList;
            for (const auto& addr : dnsResult.addresses) {
                addressList += addr + "\n";
            }
            result.data = std::vector<uint8_t>(addressList.begin(), addressList.end());
        }
        
        return result;
    });
    
    return std::make_shared<AsyncNetworkOperation>(std::move(future), NetworkOperationType::Resolve, NetworkAddress(hostname, 0));
}

std::shared_ptr<AsyncNetworkOperation> DNSResolver::reverseResolveAsync(const std::string& ipAddress) {
    auto future = NetworkRuntime::getInstance().getScheduler().schedule([ipAddress]() {
        auto dnsResult = reverseResolve(ipAddress);
        
        NetworkResult result;
        result.success = dnsResult.success;
        result.errorMessage = dnsResult.errorMessage;
        result.duration = dnsResult.duration;
        
        if (dnsResult.success && !dnsResult.addresses.empty()) {
            std::string hostname = dnsResult.addresses[0];
            result.data = std::vector<uint8_t>(hostname.begin(), hostname.end());
        }
        
        return result;
    });
    
    return std::make_shared<AsyncNetworkOperation>(std::move(future), NetworkOperationType::Resolve, NetworkAddress(ipAddress, 0));
}

DNSResolver::DNSResult DNSResolver::performResolve(const std::string& hostname, bool reverse) {
    DNSResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    if (reverse) {
        // 反向DNS解析
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        
        if (inet_pton(AF_INET, hostname.c_str(), &addr.sin_addr) <= 0) {
            result.success = false;
            result.errorMessage = "Invalid IP address";
            return result;
        }
        
        char host[NI_MAXHOST];
        int status = getnameinfo((struct sockaddr*)&addr, sizeof(addr),
                               host, NI_MAXHOST, nullptr, 0, NI_NAMEREQD);
        
        if (status == 0) {
            result.success = true;
            result.addresses.push_back(host);
        } else {
            result.success = false;
#ifdef _WIN32
            // Windows下gai_strerror返回WCHAR*，需要特殊处理
            result.errorMessage = "Reverse DNS lookup failed: error code " + std::to_string(status);
#else
            result.errorMessage = "Reverse DNS lookup failed: " + std::string(gai_strerror(status));
#endif
        }
    } else {
        // 正向DNS解析
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        int status = getaddrinfo(hostname.c_str(), nullptr, &hints, &res);
        
        if (status == 0) {
            result.success = true;
            
            for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
                struct sockaddr_in* addr_in = (struct sockaddr_in*)p->ai_addr;
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &addr_in->sin_addr, ip, INET_ADDRSTRLEN);
                result.addresses.push_back(ip);
            }
            
            freeaddrinfo(res);
        } else {
            result.success = false;
#ifdef _WIN32
            // Windows下gai_strerror返回WCHAR*，需要特殊处理
            result.errorMessage = "DNS lookup failed: error code " + std::to_string(status);
#else
            result.errorMessage = "DNS lookup failed: " + std::string(gai_strerror(status));
#endif
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    return result;
}

// NetworkRuntime 实现
std::unique_ptr<NetworkRuntime> NetworkRuntime::instance_;
std::mutex NetworkRuntime::instanceMutex_;

NetworkRuntime::NetworkRuntime() {
    scheduler_ = std::make_unique<NetworkScheduler>();
    scheduler_->start();
}

NetworkRuntime::~NetworkRuntime() {
    if (scheduler_) {
        scheduler_->stop();
    }
}

NetworkRuntime& NetworkRuntime::getInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex_);
    if (!instance_) {
        instance_ = std::unique_ptr<NetworkRuntime>(new NetworkRuntime());
    }
    return *instance_;
}

void NetworkRuntime::initialize() {
    getInstance();
}

void NetworkRuntime::shutdown() {
    std::lock_guard<std::mutex> lock(instanceMutex_);
    instance_.reset();
}

std::shared_ptr<TCPSocket> NetworkRuntime::createTCPSocket() {
    auto socket = std::make_shared<TCPSocket>();
    registerSocket(socket);
    return socket;
}

std::shared_ptr<UDPSocket> NetworkRuntime::createUDPSocket() {
    auto socket = std::make_shared<UDPSocket>();
    registerSocket(socket);
    return socket;
}

std::unique_ptr<HTTPClient> NetworkRuntime::createHTTPClient() {
    return std::make_unique<HTTPClient>();
}

std::unique_ptr<HTTPServer> NetworkRuntime::createHTTPServer() {
    return std::make_unique<HTTPServer>();
}

void NetworkRuntime::registerSocket(std::shared_ptr<Socket> socket) {
    std::lock_guard<std::mutex> lock(socketsMutex_);
    activeSockets_[socket->getSocketFd()] = socket;
}

void NetworkRuntime::unregisterSocket(socket_t sockfd) {
    std::lock_guard<std::mutex> lock(socketsMutex_);
    activeSockets_.erase(sockfd);
}

std::shared_ptr<Socket> NetworkRuntime::getSocket(socket_t sockfd) {
    std::lock_guard<std::mutex> lock(socketsMutex_);
    auto it = activeSockets_.find(sockfd);
    return (it != activeSockets_.end()) ? it->second : nullptr;
}

NetworkResult NetworkRuntime::httpGet(const std::string& url) {
    auto client = createHTTPClient();
    HTTPResponse response;
    return client->get(url, response);
}

NetworkResult NetworkRuntime::httpPost(const std::string& url, const std::string& data) {
    auto client = createHTTPClient();
    HTTPResponse response;
    return client->post(url, data, response);
}

std::shared_ptr<AsyncNetworkOperation> NetworkRuntime::httpGetAsync(const std::string& url) {
    auto client = createHTTPClient();
    return client->getAsync(url);
}

std::shared_ptr<AsyncNetworkOperation> NetworkRuntime::httpPostAsync(const std::string& url, const std::string& data) {
    auto client = createHTTPClient();
    std::vector<uint8_t> bodyData(data.begin(), data.end());
    return client->postAsync(url, bodyData);
}

DNSResolver::DNSResult NetworkRuntime::resolveHostname(const std::string& hostname) {
    return DNSResolver::resolve(hostname);
}

std::shared_ptr<AsyncNetworkOperation> NetworkRuntime::resolveHostnameAsync(const std::string& hostname) {
    return DNSResolver::resolveAsync(hostname);
}

size_t NetworkRuntime::getActiveSocketCount() const {
    std::lock_guard<std::mutex> lock(socketsMutex_);
    return activeSockets_.size();
}

std::vector<NetworkAddress> NetworkRuntime::getActiveConnections() const {
    std::lock_guard<std::mutex> lock(socketsMutex_);
    
    std::vector<NetworkAddress> connections;
    for (const auto& pair : activeSockets_) {
        if (pair.second->isConnected()) {
            connections.push_back(pair.second->getRemoteAddress());
        }
    }
    
    return connections;
}

// HTTPServer 实现
HTTPServer::HTTPServer() 
    : running_(false), maxConnections_(100), requestTimeout_(std::chrono::seconds(30)) {
}

HTTPServer::~HTTPServer() {
    stop();
}

void HTTPServer::addRoute(const std::string& path, HTTPRequestHandler handler) {
    routes_[path] = handler;
}

void HTTPServer::setDefaultHandler(HTTPRequestHandler handler) {
    defaultHandler_ = handler;
}

bool HTTPServer::start(const NetworkAddress& address) {
    if (running_.load()) {
        return false;
    }
    
    serverSocket_ = std::make_unique<TCPSocket>();
    if (!serverSocket_->bind(address) || !serverSocket_->listen()) {
        return false;
    }
    
    running_.store(true);
    
    // 启动接受连接的线程
    workerThreads_.emplace_back(&HTTPServer::acceptLoop, this);
    
    return true;
}

void HTTPServer::stop() {
    if (!running_.load()) {
        return;
    }
    
    running_.store(false);
    
    if (serverSocket_) {
        serverSocket_->close();
    }
    
    for (auto& thread : workerThreads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    workerThreads_.clear();
}

void HTTPServer::acceptLoop() {
    while (running_.load()) {
        auto clientSocket = serverSocket_->accept();
        if (clientSocket && running_.load()) {
            // 在新线程中处理客户端
            workerThreads_.emplace_back(&HTTPServer::handleClient, this, clientSocket);
        }
    }
}

void HTTPServer::handleClient(std::shared_ptr<TCPSocket> clientSocket) {
    // 简单的HTTP请求处理实现
    auto result = clientSocket->receive(4096);
    if (result.success && !result.data.empty()) {
        std::string requestData(result.data.begin(), result.data.end());
        
        HTTPRequest request;
        if (parseRequest(requestData, request)) {
            HTTPResponse response;
            response.statusCode = 200;
            response.statusMessage = "OK";
            response.version = "HTTP/1.1";
            
            auto handler = findHandler(request.path);
            if (handler) {
                handler(request, response);
            } else if (defaultHandler_) {
                defaultHandler_(request, response);
            } else {
                response.statusCode = 404;
                response.statusMessage = "Not Found";
                std::string body = "404 Not Found";
                response.body = std::vector<uint8_t>(body.begin(), body.end());
            }
            
            std::string responseStr = buildResponseString(response);
            std::vector<uint8_t> responseData(responseStr.begin(), responseStr.end());
            clientSocket->send(responseData);
        }
    }
    
    clientSocket->close();
}

bool HTTPServer::parseRequest(const std::string& requestData, HTTPRequest& request) {
    // 简单的HTTP请求解析
    std::istringstream stream(requestData);
    std::string line;
    
    if (!std::getline(stream, line)) {
        return false;
    }
    
    std::istringstream requestLine(line);
    std::string method, url, version;
    requestLine >> method >> url >> version;
    
    if (method == "GET") request.method = HTTPMethod::GET;
    else if (method == "POST") request.method = HTTPMethod::POST;
    else if (method == "PUT") request.method = HTTPMethod::PUT;
    else if (method == "DELETE") request.method = HTTPMethod::DELETE;
    else return false;
    
    request.url = url;
    request.path = url; // 简化处理
    request.version = version;
    
    return true;
}

std::string HTTPServer::buildResponseString(const HTTPResponse& response) {
    std::ostringstream stream;
    stream << response.version << " " << response.statusCode << " " << response.statusMessage << "\r\n";
    
    for (const auto& header : response.headers) {
        stream << header.first << ": " << header.second << "\r\n";
    }
    
    stream << "Content-Length: " << response.body.size() << "\r\n";
    stream << "\r\n";
    
    if (!response.body.empty()) {
        stream << std::string(response.body.begin(), response.body.end());
    }
    
    return stream.str();
}

HTTPRequestHandler HTTPServer::findHandler(const std::string& path) {
    auto it = routes_.find(path);
    return (it != routes_.end()) ? it->second : nullptr;
}

} // namespace miniswift