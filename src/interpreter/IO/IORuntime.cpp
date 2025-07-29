#include "IORuntime.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace miniswift {

// AsyncIOOperation 实现
AsyncIOOperation::AsyncIOOperation(std::future<IOResult> future, IOOperationType type, const std::string& path)
    : future_(std::move(future)), type_(type), path_(path), cancelled_(false) {
}

IOResult AsyncIOOperation::wait() {
    if (cancelled_.load()) {
        IOResult result;
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

bool AsyncIOOperation::isReady() const {
    if (cancelled_.load()) {
        return true;
    }
    return future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

void AsyncIOOperation::cancel() {
    cancelled_.store(true);
}

void AsyncIOOperation::onCompletion(CompletionHandler handler) {
    completion_ = std::move(handler);
}

// FileHandle 实现
FileHandle::FileHandle(const std::string& path, std::ios::openmode mode)
    : path_(path), mode_(mode), isOpen_(false) {
    stream_ = std::make_unique<std::fstream>();
}

FileHandle::~FileHandle() {
    if (isOpen_) {
        close();
    }
}

FileHandle::FileHandle(FileHandle&& other) noexcept
    : path_(std::move(other.path_))
    , stream_(std::move(other.stream_))
    , isOpen_(other.isOpen_)
    , mode_(other.mode_) {
    other.isOpen_ = false;
}

FileHandle& FileHandle::operator=(FileHandle&& other) noexcept {
    if (this != &other) {
        if (isOpen_) {
            close();
        }
        
        path_ = std::move(other.path_);
        stream_ = std::move(other.stream_);
        isOpen_ = other.isOpen_;
        mode_ = other.mode_;
        other.isOpen_ = false;
    }
    return *this;
}

bool FileHandle::open() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isOpen_) {
        return true;
    }
    
    try {
        stream_->open(path_, mode_);
        isOpen_ = stream_->is_open();
        return isOpen_;
    } catch (const std::exception& e) {
        isOpen_ = false;
        return false;
    }
}

void FileHandle::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isOpen_ && stream_) {
        stream_->close();
        isOpen_ = false;
    }
}

bool FileHandle::isOpen() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return isOpen_;
}

IOResult FileHandle::read(size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    IOResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    if (!isOpen_) {
        result.success = false;
        result.errorMessage = "File is not open";
        return result;
    }
    
    try {
        std::vector<uint8_t> buffer(size);
        stream_->read(reinterpret_cast<char*>(buffer.data()), size);
        
        size_t bytesRead = stream_->gcount();
        buffer.resize(bytesRead);
        
        result.success = true;
        result.data = std::move(buffer);
        result.bytesProcessed = bytesRead;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    return result;
}

IOResult FileHandle::readAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    IOResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    if (!isOpen_) {
        result.success = false;
        result.errorMessage = "File is not open";
        return result;
    }
    
    try {
        // 获取文件大小
        stream_->seekg(0, std::ios::end);
        size_t fileSize = stream_->tellg();
        stream_->seekg(0, std::ios::beg);
        
        std::vector<uint8_t> buffer(fileSize);
        stream_->read(reinterpret_cast<char*>(buffer.data()), fileSize);
        
        size_t bytesRead = stream_->gcount();
        buffer.resize(bytesRead);
        
        result.success = true;
        result.data = std::move(buffer);
        result.bytesProcessed = bytesRead;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    return result;
}

IOResult FileHandle::write(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    IOResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    if (!isOpen_) {
        result.success = false;
        result.errorMessage = "File is not open";
        return result;
    }
    
    try {
        stream_->write(reinterpret_cast<const char*>(data.data()), data.size());
        stream_->flush();
        
        result.success = !stream_->fail();
        result.bytesProcessed = data.size();
        
        if (!result.success) {
            result.errorMessage = "Write operation failed";
        }
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    return result;
}

IOResult FileHandle::write(const std::string& text) {
    std::vector<uint8_t> data(text.begin(), text.end());
    return write(data);
}

std::shared_ptr<AsyncIOOperation> FileHandle::readAsync(size_t size) {
    auto future = IORuntime::getInstance().getScheduler().schedule([this, size]() {
        return this->read(size);
    });
    
    return std::make_shared<AsyncIOOperation>(std::move(future), IOOperationType::Read, path_);
}

std::shared_ptr<AsyncIOOperation> FileHandle::writeAsync(const std::vector<uint8_t>& data) {
    auto future = IORuntime::getInstance().getScheduler().schedule([this, data]() {
        return this->write(data);
    });
    
    return std::make_shared<AsyncIOOperation>(std::move(future), IOOperationType::Write, path_);
}

size_t FileHandle::size() const {
    try {
        return std::filesystem::file_size(path_);
    } catch (const std::exception&) {
        return 0;
    }
}

std::filesystem::file_time_type FileHandle::lastModified() const {
    try {
        return std::filesystem::last_write_time(path_);
    } catch (const std::exception&) {
        return std::filesystem::file_time_type{};
    }
}

bool FileHandle::exists() const {
    return std::filesystem::exists(path_);
}

// DirectoryHandle 实现
DirectoryHandle::DirectoryHandle(const std::string& path) : path_(path) {
}

bool DirectoryHandle::create(bool recursive) {
    try {
        if (recursive) {
            return std::filesystem::create_directories(path_);
        } else {
            return std::filesystem::create_directory(path_);
        }
    } catch (const std::exception&) {
        return false;
    }
}

bool DirectoryHandle::remove(bool recursive) {
    try {
        if (recursive) {
            return std::filesystem::remove_all(path_) > 0;
        } else {
            return std::filesystem::remove(path_);
        }
    } catch (const std::exception&) {
        return false;
    }
}

bool DirectoryHandle::exists() const {
    return std::filesystem::exists(path_) && std::filesystem::is_directory(path_);
}

std::vector<std::string> DirectoryHandle::list() const {
    std::vector<std::string> result;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path_)) {
            result.push_back(entry.path().filename().string());
        }
    } catch (const std::exception&) {
        // 返回空列表
    }
    
    return result;
}

std::vector<std::string> DirectoryHandle::listFiles() const {
    std::vector<std::string> result;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path_)) {
            if (entry.is_regular_file()) {
                result.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::exception&) {
        // 返回空列表
    }
    
    return result;
}

std::vector<std::string> DirectoryHandle::listDirectories() const {
    std::vector<std::string> result;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path_)) {
            if (entry.is_directory()) {
                result.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::exception&) {
        // 返回空列表
    }
    
    return result;
}

std::shared_ptr<AsyncIOOperation> DirectoryHandle::listAsync() const {
    auto future = IORuntime::getInstance().getScheduler().schedule([this]() {
        IOResult result;
        auto files = this->list();
        
        // 将文件列表转换为字符串数据
        std::ostringstream oss;
        for (const auto& file : files) {
            oss << file << "\n";
        }
        
        std::string listStr = oss.str();
        result.data = std::vector<uint8_t>(listStr.begin(), listStr.end());
        result.success = true;
        result.bytesProcessed = result.data.size();
        
        return result;
    });
    
    return std::make_shared<AsyncIOOperation>(std::move(future), IOOperationType::List, path_);
}

// IOBuffer 实现
IOBuffer::IOBuffer(size_t initialSize) : readPos_(0), writePos_(0) {
    buffer_.resize(initialSize);
}

size_t IOBuffer::write(const uint8_t* data, size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t available = buffer_.size() - writePos_;
    size_t toWrite = std::min(size, available);
    
    if (toWrite > 0) {
        std::copy(data, data + toWrite, buffer_.begin() + writePos_);
        writePos_ += toWrite;
    }
    
    return toWrite;
}

size_t IOBuffer::read(uint8_t* data, size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t available = writePos_ - readPos_;
    size_t toRead = std::min(size, available);
    
    if (toRead > 0) {
        std::copy(buffer_.begin() + readPos_, buffer_.begin() + readPos_ + toRead, data);
        readPos_ += toRead;
    }
    
    return toRead;
}

size_t IOBuffer::available() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return writePos_ - readPos_;
}

size_t IOBuffer::capacity() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_.size();
}

bool IOBuffer::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return readPos_ == writePos_;
}

bool IOBuffer::full() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return writePos_ == buffer_.size();
}

void IOBuffer::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    readPos_ = 0;
    writePos_ = 0;
}

void IOBuffer::resize(size_t newSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_.resize(newSize);
    
    // 调整位置指针
    if (readPos_ > newSize) readPos_ = newSize;
    if (writePos_ > newSize) writePos_ = newSize;
}

void IOBuffer::compact() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (readPos_ > 0) {
        size_t dataSize = writePos_ - readPos_;
        if (dataSize > 0) {
            std::copy(buffer_.begin() + readPos_, buffer_.begin() + writePos_, buffer_.begin());
        }
        readPos_ = 0;
        writePos_ = dataSize;
    }
}

// IOScheduler 实现
IOScheduler::IOScheduler(size_t numThreads) : running_(false) {
    workers_.reserve(numThreads);
}

IOScheduler::~IOScheduler() {
    stop();
}

void IOScheduler::start() {
    if (running_.load()) {
        return;
    }
    
    running_.store(true);
    
    size_t numThreads = workers_.capacity();
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back(&IOScheduler::workerLoop, this);
    }
}

void IOScheduler::stop() {
    if (!running_.load()) {
        return;
    }
    
    running_.store(false);
    queueCondition_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
}

size_t IOScheduler::queueSize() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return operationQueue_.size();
}

void IOScheduler::workerLoop() {
    while (running_.load()) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCondition_.wait(lock, [this] {
                return !operationQueue_.empty() || !running_.load();
            });
            
            if (!running_.load()) {
                break;
            }
            
            if (!operationQueue_.empty()) {
                task = std::move(operationQueue_.front());
                operationQueue_.pop();
            }
        }
        
        if (task) {
            try {
                task();
            } catch (const std::exception& e) {
                // 记录错误但继续运行
                std::cerr << "IOScheduler task error: " << e.what() << std::endl;
            }
        }
    }
}

// IORuntime 实现
std::unique_ptr<IORuntime> IORuntime::instance_;
std::mutex IORuntime::instanceMutex_;

IORuntime::IORuntime() {
    scheduler_ = std::make_unique<IOScheduler>();
    scheduler_->start();
}

IORuntime::~IORuntime() {
    if (scheduler_) {
        scheduler_->stop();
    }
}

IORuntime& IORuntime::getInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex_);
    if (!instance_) {
        instance_ = std::unique_ptr<IORuntime>(new IORuntime());
    }
    return *instance_;
}

void IORuntime::initialize() {
    getInstance(); // 确保实例被创建
}

void IORuntime::shutdown() {
    std::lock_guard<std::mutex> lock(instanceMutex_);
    instance_.reset();
}

std::shared_ptr<FileHandle> IORuntime::openFile(const std::string& path, std::ios::openmode mode) {
    std::lock_guard<std::mutex> lock(filesMutex_);
    
    auto it = openFiles_.find(path);
    if (it != openFiles_.end()) {
        return it->second;
    }
    
    auto handle = std::make_shared<FileHandle>(path, mode);
    if (handle->open()) {
        openFiles_[path] = handle;
        return handle;
    }
    
    return nullptr;
}

void IORuntime::closeFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(filesMutex_);
    
    auto it = openFiles_.find(path);
    if (it != openFiles_.end()) {
        it->second->close();
        openFiles_.erase(it);
    }
}

std::unique_ptr<DirectoryHandle> IORuntime::openDirectory(const std::string& path) {
    return std::make_unique<DirectoryHandle>(path);
}

IOResult IORuntime::readFile(const std::string& path) {
    auto handle = openFile(path, std::ios::in | std::ios::binary);
    if (!handle) {
        IOResult result;
        result.success = false;
        result.errorMessage = "Failed to open file: " + path;
        return result;
    }
    
    auto result = handle->readAll();
    closeFile(path);
    return result;
}

IOResult IORuntime::writeFile(const std::string& path, const std::vector<uint8_t>& data) {
    auto handle = openFile(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!handle) {
        IOResult result;
        result.success = false;
        result.errorMessage = "Failed to open file: " + path;
        return result;
    }
    
    auto result = handle->write(data);
    closeFile(path);
    return result;
}

IOResult IORuntime::writeFile(const std::string& path, const std::string& text) {
    std::vector<uint8_t> data(text.begin(), text.end());
    return writeFile(path, data);
}

std::shared_ptr<AsyncIOOperation> IORuntime::readFileAsync(const std::string& path) {
    auto future = scheduler_->schedule([this, path]() {
        return this->readFile(path);
    });
    
    return std::make_shared<AsyncIOOperation>(std::move(future), IOOperationType::Read, path);
}

std::shared_ptr<AsyncIOOperation> IORuntime::writeFileAsync(const std::string& path, const std::vector<uint8_t>& data) {
    auto future = scheduler_->schedule([this, path, data]() {
        return this->writeFile(path, data);
    });
    
    return std::make_shared<AsyncIOOperation>(std::move(future), IOOperationType::Write, path);
}

bool IORuntime::copyFile(const std::string& source, const std::string& destination) {
    try {
        return std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing);
    } catch (const std::exception&) {
        return false;
    }
}

bool IORuntime::moveFile(const std::string& source, const std::string& destination) {
    try {
        std::filesystem::rename(source, destination);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool IORuntime::deleteFile(const std::string& path) {
    try {
        return std::filesystem::remove(path);
    } catch (const std::exception&) {
        return false;
    }
}

bool IORuntime::fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

} // namespace miniswift