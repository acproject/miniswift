#include "IONetworkIntegration.h"
#include "Environment.h"
#include "Interpreter.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <algorithm>

namespace miniswift {

// 获取IO运行时实例
static IORuntime& getIORuntime() {
    return IORuntime::getInstance();
}

// 获取Network运行时实例
static NetworkRuntime& getNetworkRuntime() {
    return NetworkRuntime::getInstance();
}

// IONetworkBridge 实现
Value IONetworkBridge::readFile(const std::string& path) {
    auto& runtime = getIORuntime();
    auto result = runtime.readFile(path);
    return convertIOResultToValue(result);
}

Value IONetworkBridge::writeFile(const std::string& path, const Value& content) {
    auto& runtime = getIORuntime();
    auto data = valueToByteArray(content);
    auto result = runtime.writeFile(path, data);
    return convertIOResultToValue(result);
}

Value IONetworkBridge::fileExists(const std::string& path) {
    try {
        bool exists = std::filesystem::exists(path);
        return Value(exists);
    } catch (const std::exception& e) {
        return createIOError("Error checking file existence: " + std::string(e.what()));
    }
}

Value IONetworkBridge::deleteFile(const std::string& path) {
    try {
        bool deleted = std::filesystem::remove(path);
        return Value(deleted);
    } catch (const std::exception& e) {
        return createIOError("Error deleting file: " + std::string(e.what()));
    }
}

Value IONetworkBridge::copyFile(const std::string& source, const std::string& destination) {
    try {
        std::filesystem::copy_file(source, destination);
        return Value(true);
    } catch (const std::exception& e) {
        return createIOError("Error copying file: " + std::string(e.what()));
    }
}

Value IONetworkBridge::moveFile(const std::string& source, const std::string& destination) {
    try {
        std::filesystem::rename(source, destination);
        return Value(true);
    } catch (const std::exception& e) {
        return createIOError("Error moving file: " + std::string(e.what()));
    }
}

Value IONetworkBridge::readFileAsync(const std::string& path) {
    auto& runtime = getIORuntime();
    auto operation = runtime.readFileAsync(path);
    
    // 返回一个包含操作指针的值
    return Value(static_cast<int64_t>(reinterpret_cast<uintptr_t>(operation.get())));
}

Value IONetworkBridge::writeFileAsync(const std::string& path, const Value& content) {
    auto& runtime = getIORuntime();
    auto data = valueToByteArray(content);
    auto operation = runtime.writeFileAsync(path, data);
    
    // 返回一个包含操作指针的值
    return Value(static_cast<int64_t>(reinterpret_cast<uintptr_t>(operation.get())));
}

Value IONetworkBridge::createDirectory(const std::string& path, bool recursive) {
    try {
        bool created;
        if (recursive) {
            created = std::filesystem::create_directories(path);
        } else {
            created = std::filesystem::create_directory(path);
        }
        return Value(created);
    } catch (const std::exception& e) {
        return createIOError("Error creating directory: " + std::string(e.what()));
    }
}

Value IONetworkBridge::removeDirectory(const std::string& path, bool recursive) {
    try {
        std::uintmax_t removed;
        if (recursive) {
            removed = std::filesystem::remove_all(path);
        } else {
            removed = std::filesystem::remove(path) ? 1 : 0;
        }
        return Value(static_cast<int>(removed));
    } catch (const std::exception& e) {
        return createIOError("Error removing directory: " + std::string(e.what()));
    }
}

Value IONetworkBridge::listDirectory(const std::string& path) {
    try {
        auto arrayPtr = std::make_shared<std::vector<Value>>();
        
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            Value item(entry.path().filename().string());
            arrayPtr->push_back(item);
        }
        
        return Value(arrayPtr);
    } catch (const std::exception& e) {
        return createIOError("Error listing directory: " + std::string(e.what()));
    }
}

Value IONetworkBridge::directoryExists(const std::string& path) {
    try {
        bool exists = std::filesystem::exists(path) && std::filesystem::is_directory(path);
        return Value(exists);
    } catch (const std::exception& e) {
        return createIOError("Error checking directory existence: " + std::string(e.what()));
    }
}

Value IONetworkBridge::httpGet(const std::string& url) {
    auto& runtime = getNetworkRuntime();
    auto client = runtime.createHTTPClient();
    if (!client) {
        return createNetworkError("Failed to create HTTP client");
    }
    
    HTTPRequest request;
    request.method = HTTPMethod::GET;
    request.url = url;
    
    HTTPResponse response;
    auto result = client->request(request, response);
    return convertNetworkResultToValue(result);
}

Value IONetworkBridge::httpPost(const std::string& url, const Value& data) {
    auto& runtime = getNetworkRuntime();
    auto client = runtime.createHTTPClient();
    if (!client) {
        return createNetworkError("Failed to create HTTP client");
    }
    
    HTTPRequest request;
    request.method = HTTPMethod::POST;
    request.url = url;
    request.body = valueToByteArray(data);
    
    HTTPResponse response;
    auto result = client->request(request, response);
    return convertNetworkResultToValue(result);
}

Value IONetworkBridge::createIOError(const std::string& message) {
    // 创建一个简单的字符串错误表示
    return Value("IOError: " + message);
}

Value IONetworkBridge::createNetworkError(const std::string& message) {
    // 创建一个简单的字符串错误表示
    return Value("NetworkError: " + message);
}

Value IONetworkBridge::createSuccessResult(const Value& data) {
    // 对于成功结果，直接返回数据
    return data;
}

Value IONetworkBridge::convertIOResultToValue(const IOResult& result) {
    if (result.success) {
        return byteArrayToValue(result.data);
    } else {
        return createIOError(result.errorMessage);
    }
}

Value IONetworkBridge::convertNetworkResultToValue(const NetworkResult& result) {
    if (result.success) {
        return byteArrayToValue(result.data);
    } else {
        return createNetworkError(result.errorMessage);
    }
}

std::vector<uint8_t> IONetworkBridge::valueToByteArray(const Value& value) {
    std::vector<uint8_t> result;
    
    if (value.type == ValueType::String) {
        const std::string& str = std::get<std::string>(value.value);
        result.assign(str.begin(), str.end());
    } else {
        // 对于其他类型，转换为字符串表示
        std::string str = "[object]";
        result.assign(str.begin(), str.end());
    }
    
    return result;
}

Value IONetworkBridge::byteArrayToValue(const std::vector<uint8_t>& data) {
    std::string str(data.begin(), data.end());
    return Value(str);
}

// AsyncOperationWrapper 实现
AsyncOperationWrapper::AsyncOperationWrapper(std::shared_ptr<AsyncIOOperation> op)
    : type_(OperationType::IO_READ), ioOperation_(op) {}

AsyncOperationWrapper::AsyncOperationWrapper(std::shared_ptr<AsyncNetworkOperation> op)
    : type_(OperationType::NETWORK_REQUEST), networkOperation_(op) {}

Value AsyncOperationWrapper::wait() {
    switch (type_) {
        case OperationType::IO_READ:
        case OperationType::IO_WRITE: {
            if (ioOperation_) {
                auto result = ioOperation_->wait();
                return IONetworkBridge::convertIOResultToValue(result);
            }
            break;
        }
        case OperationType::NETWORK_REQUEST:
        case OperationType::DNS_RESOLVE: {
            if (networkOperation_) {
                auto result = networkOperation_->wait();
                return IONetworkBridge::convertNetworkResultToValue(result);
            }
            break;
        }
    }
    
    return IONetworkBridge::createIOError("Invalid async operation");
}

bool AsyncOperationWrapper::isReady() const {
    switch (type_) {
        case OperationType::IO_READ:
        case OperationType::IO_WRITE:
            return ioOperation_ && ioOperation_->isReady();
        case OperationType::NETWORK_REQUEST:
        case OperationType::DNS_RESOLVE:
            return networkOperation_ && networkOperation_->isReady();
    }
    return false;
}

void AsyncOperationWrapper::cancel() {
    switch (type_) {
        case OperationType::IO_READ:
        case OperationType::IO_WRITE:
            if (ioOperation_) {
                ioOperation_->cancel();
            }
            break;
        case OperationType::NETWORK_REQUEST:
        case OperationType::DNS_RESOLVE:
            if (networkOperation_) {
                networkOperation_->cancel();
            }
            break;
    }
}

void AsyncOperationWrapper::onCompletion(std::function<void(const Value&)> callback) {
    completion_ = callback;
}

// HTTPClientWrapper 实现
HTTPClientWrapper::HTTPClientWrapper() {
    auto& runtime = getNetworkRuntime();
    client_ = runtime.createHTTPClient();
}

void HTTPClientWrapper::setTimeout(int milliseconds) {
    if (client_) {
        client_->setTimeout(std::chrono::milliseconds{milliseconds});
    }
}

void HTTPClientWrapper::setHeader(const std::string& name, const std::string& value) {
    if (client_) {
        client_->setDefaultHeader(name, value);
    }
}

Value HTTPClientWrapper::get(const std::string& url) {
    if (!client_) {
        return IONetworkBridge::createNetworkError("HTTP client not initialized");
    }
    
    HTTPRequest request;
    request.method = HTTPMethod::GET;
    request.url = url;
    
    HTTPResponse response;
    auto result = client_->request(request, response);
    return IONetworkBridge::convertNetworkResultToValue(result);
}

Value HTTPClientWrapper::post(const std::string& url, const Value& data) {
    if (!client_) {
        return IONetworkBridge::createNetworkError("HTTP client not initialized");
    }
    
    HTTPRequest request;
    request.method = HTTPMethod::POST;
    request.url = url;
    request.body = IONetworkBridge::valueToByteArray(data);
    
    HTTPResponse response;
    auto result = client_->request(request, response);
    return IONetworkBridge::convertNetworkResultToValue(result);
}

// IONetworkRegistry 实现
void IONetworkRegistry::registerIOFunctions(Environment& globalEnv) {
    registerFileOperations(globalEnv);
    registerDirectoryOperations(globalEnv);
}

void IONetworkRegistry::registerNetworkFunctions(Environment& globalEnv) {
    registerNetworkOperations(globalEnv);
}

void IONetworkRegistry::registerFileOperations(Environment& env) {
    // 这里可以注册文件操作函数到环境中
    // 由于Value和Environment的具体实现限制，这里只是占位符
}

void IONetworkRegistry::registerDirectoryOperations(Environment& env) {
    // 注册目录操作函数
}

void IONetworkRegistry::registerNetworkOperations(Environment& env) {
    // 注册网络操作函数
}

void IONetworkRegistry::initialize() {
    // 初始化全局运行时
    getIORuntime();
    getNetworkRuntime();
}

void IONetworkRegistry::shutdown() {
    IORuntime::shutdown();
    NetworkRuntime::shutdown();
}

// 全局初始化函数
void initializeIONetworkSupport() {
    IONetworkRegistry::initialize();
}

void shutdownIONetworkSupport() {
    IONetworkRegistry::shutdown();
}

} // namespace miniswift