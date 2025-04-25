#include <iostream>
#include "IOServicePool.h"


IOServicePool::IOServicePool(std::size_t pool_size)
        : _ioServices(pool_size), _works(pool_size), _nextIOServiceIndex(0) {
    for (std::size_t i = 0; i < pool_size; ++i) {
        _works[i] = std::unique_ptr<Work>(new Work(_ioServices[i].get_executor()));
    }

    // 遍历ioservice 启动线程 ，每个线程内部启动ioservice
    // ;此处建议二级构造，而非直接在构造函数中启动线程
    for (std::size_t i = 0; i < _ioServices.size(); ++i) {
        _threads.emplace_back([this, i]() { _ioServices[i].run(); });
    }
}


IOServicePool::~IOServicePool() { std::cout << "~IOServicePool()" << std::endl; }

IOServicePool& IOServicePool::Instance() {
    //简易单例模式 ，在cpp11后，局部静态变量是线程安全的
    static IOServicePool instance;
    return instance;
}


boost::asio::io_context& IOServicePool::GetIOService() {
    return _ioServices[_nextIOServiceIndex++ % _ioServices.size()];
}

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
