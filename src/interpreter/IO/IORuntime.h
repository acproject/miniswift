#ifndef MINISWIFT_IO_RUNTIME_H
#define MINISWIFT_IO_RUNTIME_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <atomic>
#include "../Value.h"
#include "../Concurrency/ConcurrencyRuntime.h"
#include "../Memory/MemoryManager.h"

namespace miniswift {

// Forward declarations
class FileHandle;
class DirectoryHandle;
class IOBuffer;
class IOScheduler;

// IO操作类型
enum class IOOperationType {
    Read,
    Write,
    Append,
    Create,
    Delete,
    Copy,
    Move,
    List
};

// IO操作结果
struct IOResult {
    bool success;
    std::string errorMessage;
    size_t bytesProcessed;
    std::vector<uint8_t> data;
    std::chrono::milliseconds duration;
    
    IOResult() : success(false), bytesProcessed(0) {}
    IOResult(bool s, const std::string& err = "") : success(s), errorMessage(err), bytesProcessed(0) {}
};

// 异步IO操作句柄
class AsyncIOOperation {
public:
    using CompletionHandler = std::function<void(const IOResult&)>;
    
private:
    std::future<IOResult> future_;
    CompletionHandler completion_;
    std::atomic<bool> cancelled_;
    IOOperationType type_;
    std::string path_;
    
public:
    AsyncIOOperation(std::future<IOResult> future, IOOperationType type, const std::string& path);
    
    // 等待操作完成
    IOResult wait();
    
    // 检查操作是否完成
    bool isReady() const;
    
    // 取消操作
    void cancel();
    
    // 设置完成回调
    void onCompletion(CompletionHandler handler);
    
    // 获取操作信息
    IOOperationType getType() const { return type_; }
    const std::string& getPath() const { return path_; }
    bool isCancelled() const { return cancelled_.load(); }
};

// 文件句柄类
class FileHandle {
private:
    std::string path_;
    std::unique_ptr<std::fstream> stream_;
    bool isOpen_;
    std::ios::openmode mode_;
    mutable std::mutex mutex_;
    
public:
    FileHandle(const std::string& path, std::ios::openmode mode);
    ~FileHandle();
    
    // 禁止拷贝，允许移动
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    FileHandle(FileHandle&& other) noexcept;
    FileHandle& operator=(FileHandle&& other) noexcept;
    
    // 文件操作
    bool open();
    void close();
    bool isOpen() const;
    
    // 同步读写
    IOResult read(size_t size);
    IOResult readAll();
    IOResult write(const std::vector<uint8_t>& data);
    IOResult write(const std::string& text);
    
    // 异步读写
    std::shared_ptr<AsyncIOOperation> readAsync(size_t size);
    std::shared_ptr<AsyncIOOperation> writeAsync(const std::vector<uint8_t>& data);
    
    // 文件信息
    size_t size() const;
    std::filesystem::file_time_type lastModified() const;
    bool exists() const;
    
    const std::string& getPath() const { return path_; }
};

// 目录句柄类
class DirectoryHandle {
private:
    std::string path_;
    
public:
    explicit DirectoryHandle(const std::string& path);
    
    // 目录操作
    bool create(bool recursive = false);
    bool remove(bool recursive = false);
    bool exists() const;
    
    // 列出内容
    std::vector<std::string> list() const;
    std::vector<std::string> listFiles() const;
    std::vector<std::string> listDirectories() const;
    
    // 异步操作
    std::shared_ptr<AsyncIOOperation> listAsync() const;
    
    const std::string& getPath() const { return path_; }
};

// IO缓冲区
class IOBuffer {
private:
    std::vector<uint8_t> buffer_;
    size_t readPos_;
    size_t writePos_;
    mutable std::mutex mutex_;
    
public:
    IOBuffer(size_t initialSize = 4096);
    
    // 缓冲区操作
    size_t write(const uint8_t* data, size_t size);
    size_t read(uint8_t* data, size_t size);
    
    // 缓冲区状态
    size_t available() const;
    size_t capacity() const;
    bool empty() const;
    bool full() const;
    
    // 缓冲区管理
    void clear();
    void resize(size_t newSize);
    void compact(); // 压缩缓冲区，移除已读数据
};

// IO调度器
class IOScheduler {
private:
    std::unique_ptr<ThreadPool> threadPool_;
    std::queue<std::function<void()>> operationQueue_;
    mutable std::mutex queueMutex_;
    std::condition_variable queueCondition_;
    std::atomic<bool> running_;
    std::vector<std::thread> workers_;
    
public:
    IOScheduler(size_t numThreads = std::thread::hardware_concurrency());
    ~IOScheduler();
    
    // 调度操作
    template<typename Func>
    auto schedule(Func&& func) -> std::future<decltype(func())>;
    
    // 启动和停止
    void start();
    void stop();
    
    // 状态查询
    bool isRunning() const { return running_.load(); }
    size_t queueSize() const;
    
private:
    void workerLoop();
};

// 主IO运行时类
class IORuntime {
private:
    static std::unique_ptr<IORuntime> instance_;
    static std::mutex instanceMutex_;
    
    std::unique_ptr<IOScheduler> scheduler_;
    std::unordered_map<std::string, std::shared_ptr<FileHandle>> openFiles_;
    std::mutex filesMutex_;
    
    IORuntime();
    
public:
    ~IORuntime();
    
    // 单例访问
    static IORuntime& getInstance();
    static void initialize();
    static void shutdown();
    
    // 文件操作
    std::shared_ptr<FileHandle> openFile(const std::string& path, std::ios::openmode mode);
    void closeFile(const std::string& path);
    
    // 目录操作
    std::unique_ptr<DirectoryHandle> openDirectory(const std::string& path);
    
    // 便捷方法
    IOResult readFile(const std::string& path);
    IOResult writeFile(const std::string& path, const std::vector<uint8_t>& data);
    IOResult writeFile(const std::string& path, const std::string& text);
    
    // 异步便捷方法
    std::shared_ptr<AsyncIOOperation> readFileAsync(const std::string& path);
    std::shared_ptr<AsyncIOOperation> writeFileAsync(const std::string& path, const std::vector<uint8_t>& data);
    
    // 文件系统操作
    bool copyFile(const std::string& source, const std::string& destination);
    bool moveFile(const std::string& source, const std::string& destination);
    bool deleteFile(const std::string& path);
    bool fileExists(const std::string& path);
    
    // 获取调度器
    IOScheduler& getScheduler() { return *scheduler_; }
};

// 模板实现
template<typename Func>
auto IOScheduler::schedule(Func&& func) -> std::future<decltype(func())> {
    using ReturnType = decltype(func());
    
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::forward<Func>(func)
    );
    
    std::future<ReturnType> result = task->get_future();
    
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        if (!running_.load()) {
            throw std::runtime_error("IOScheduler is not running");
        }
        
        operationQueue_.emplace([task]() {
            (*task)();
        });
    }
    
    queueCondition_.notify_one();
    return result;
}

} // namespace miniswift

#endif // MINISWIFT_IO_RUNTIME_H