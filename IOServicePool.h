#ifndef IOSERVICEPOOL_H
#define IOSERVICEPOOL_H


#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>

#pragma once

class IOServicePool
{
public:
    using IOService = boost::asio::io_context;
    using IOServicePtr = std::shared_ptr<IOService>;
    using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    using WorkPtr = std::unique_ptr<Work>;    

    ~IOServicePool();
    IOServicePool(const IOServicePool&) = delete;
    IOServicePool(IOServicePool&&) = delete;
    IOServicePool& operator=(const IOServicePool&) = delete;

    //获取io_context对象
    boost::asio::io_context& GetIOService();

    void Stop();

    //获取单例对象
    //简易单例模式 ，在cpp11后，局部静态变量是线程安全的
    static IOServicePool& Instance();

private:
    IOServicePool(std::size_t pool_size = std::thread::hardware_concurrency());

    std::vector<IOService> _ioServices;
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _threads;
    // std::size_t _nextIOServiceIndex;
    std::atomic<std::size_t> _nextIOServiceIndex;
};

#endif 