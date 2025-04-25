#ifndef CSESSION_H
#define CSESSION_H

#include "MsgNode.h"
#include <memory>
#include <queue>
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/detached.hpp>




// #include "CService.h"
class CService;  // 防止循环引用


#pragma once

class CSession : public std::enable_shared_from_this<CSession> {
public:
    CSession(boost::asio::io_context& ioc, CService* service);
    ~CSession();
    boost::asio::ip::tcp::socket& GetSocket() { return _socket; }  // 获取socket对象

    void Start();

    void Close();

    void Send(const std::string& msg,short msgId);

    void Send(const char* msg, short len, short msgId);

    void Receive();
    // void HandleReceive(const boost::system::error_code& ec, std::size_t bytes_transferred);

    void HandleSend(const boost::system::error_code& ec, std::size_t bytes_transferred,std::shared_ptr<CSession> self);

    // 处理接收数据的回调函数   
    std::string GetSessionId() {
        // 生成一个唯一的会话ID
        return _sessionId;
    }

private:
    boost::asio::io_context& _ioc;
    CService* _service;
    boost::asio::ip::tcp::socket _socket;  // socket对象
    std::string _sessionId;
    std::mutex _send_mutex;
    std::queue<std::shared_ptr<SendNode> > _send_que;

    std::shared_ptr<RecvNode> _recv_data_node;
    std::shared_ptr<MsgNode> _recv_header_node;

    bool _b_closed;
};


class LogicNode{
    public:
        LogicNode(std::shared_ptr<CSession> session,std::shared_ptr<RecvNode> recvNode)
        :_session(session)
        ,_recvNode(recvNode)
        {}
        ~LogicNode(){}
        std::shared_ptr<CSession> _session;
        std::shared_ptr<RecvNode> _recvNode;

};





#endif