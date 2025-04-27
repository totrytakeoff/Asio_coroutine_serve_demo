#include <iostream>
#include "IOServicePool.h"

// 严重程度：CRITICAL
// 问题描述：构造函数中直接启动线程，不符合RAII原则，可能导致资源泄漏
// 优化建议：使用二级构造模式，将线程启动逻辑移到单独的Start()方法中
IOServicePool::IOServicePool(std::size_t pool_size)
        : _ioServices(pool_size), _works(pool_size), _nextIOServiceIndex(0) {
    for (std::size_t i = 0; i < pool_size; ++i) {
        _works[i] = std::unique_ptr<Work>(new Work(_ioServices[i].get_executor()));
    }

    // 遍历ioservice 启动线程 ，每个线程内部启动ioservice
    for (std::size_t i = 0; i < _ioServices.size(); ++i) {
        _threads.emplace_back([this, i]() { _ioServices[i].run(); });
    }
}

// 严重程度：CRITICAL
// 问题描述：析构函数中没有确保所有资源被正确清理
// 优化建议：添加资源清理检查，确保所有线程都已join
IOServicePool::~IOServicePool() { 
    std::cout << "~IOServicePool()" << std::endl; 
    Stop();
}

IOServicePool& IOServicePool::Instance() {
    //简易单例模式 ，在cpp11后，局部静态变量是线程安全的
    static IOServicePool instance;
    return instance;
}

// 严重程度：CRITICAL
// 问题描述：_nextIOServiceIndex的访问不是线程安全的，可能导致索引越界
// 优化建议：使用原子操作保护_nextIOServiceIndex的访问
boost::asio::io_context& IOServicePool::GetIOService() {
    return _ioServices[_nextIOServiceIndex++ % _ioServices.size()];
}

// 严重程度：HIGH
// 问题描述：Stop()方法没有处理异常情况
// 优化建议：添加异常处理，确保资源正确释放
void IOServicePool::Stop() {
    for(auto & work : _works) {
        work.reset();
    }
    for (auto& thread : _threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
