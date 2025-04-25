#ifndef CSERVICE_H
#define CSERVICE_H


#include <map>
#include <memory>
#include <mutex>
#include "CSession.h"
#include "IOServicePool.h"


#pragma once

class CService {
public:
    CService(boost::asio::io_context& ioc, short port);
    ~CService();

    // 移除session 
    void ClearSession(const std::string& sessionId);
private:
    // 启动接受连接
    void StartAccept();
    // 处理接受连接的结果 
    void HandleAccept(std::shared_ptr<CSession> session, const boost::system::error_code& ec);

private:
    boost::asio::io_context& _ioc;             // 用于存储IOServicePool对象的引用
    short _port;                               // 端口号
    boost::asio::ip::tcp::acceptor _acceptor;  // 接受连接的对象
    std::map<std::string, std::shared_ptr<CSession> > _sessions;  // 存储所有连接的会话对象
    std::mutex _mutex;  // 互斥锁，用于保护_sessions的访问
};

#endif