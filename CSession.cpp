#include "CService.h"
#include "CSession.h"

#include <boost/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include "LogicSystem.h"
CSession::CSession(boost::asio::io_context& ioc, CService* service)
        : _ioc(ioc), _service(service), _socket(ioc), _b_closed(false) {
    // 生成一个唯一的会话ID
    // 使用UUID生成器生成一个随机的UUID
    auto uuid = boost::uuids::random_generator()();
    _sessionId = boost::uuids::to_string(uuid);

    // 初始化接收数据头节点
    _recv_header_node = std::make_shared<MsgNode>(HEADER_LENGTH);
}


CSession::~CSession() {
    try {
        std::cout << "CSession::~CSession()" << std::endl;
        Close();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}


void CSession::Start() {
    auto self = shared_from_this();
    boost::asio::co_spawn(
            _ioc,
            [this, self]() -> boost::asio::awaitable<void> {
                try {
                    while (!_b_closed) {
                        _recv_header_node->ClearNode();
                        // 读取数据头
                        std::size_t size = co_await boost::asio::async_read(
                                _socket,
                                boost::asio::buffer(_recv_header_node->_data, HEADER_LENGTH),
                                boost::asio::use_awaitable);

                        if (size == 0) {
                            std::cout << "size == 0" << std::endl;
                            // 断开连接
                            Close();
                            co_return;

                            // _service->ClearSession(_sessionId);
                        }
                        // 处理接收数据
                        short msg_id;
                        short msg_len;
                        memcpy(&msg_id, _recv_header_node->_data, HEADER_ID_LENGTH);
                        memcpy(&msg_len,
                               _recv_header_node->_data + HEADER_ID_LENGTH,
                               HEADER_DATA_LENGTH);
                        // 字节序转换
                        msg_id = ntohs(msg_id);
                        msg_len = ntohs(msg_len);
                        // 检查数据头
                        if (msg_id > MAX_LENGTH || msg_len > MAX_LENGTH) {
                            std::cout << "invalid msg header: msg_id:" << msg_id
                                      << " msg_len:" << msg_len << std::endl;
                            Close();
                            co_return;

                            // _service->ClearSession(_sessionId);
                        }

                        // 读取数据
                        _recv_data_node = std::make_shared<RecvNode>(msg_len, msg_id);
                        size = co_await boost::asio::async_read(
                                _socket,
                                boost::asio::buffer(_recv_data_node->_data, msg_len),
                                boost::asio::use_awaitable);

                        if (size != msg_len) {
                            std::cout << "read data error" << std::endl;
                            Close();
                            // _service->ClearSession(_sessionId);
                            co_return;
                        }


                        // 处理接收数据
                        std::cout << "recv data: " << std::string(_recv_data_node->_data, msg_len)
                                  << std::endl;
                        // 构造logicNode，投递逻辑队列处理
                        auto msg = std::make_shared<LogicNode>(shared_from_this(), _recv_data_node);
                        LogicSystem::GetInstance().PostMsgtToQue(msg);
                        _recv_data_node->ClearNode();
                        // 处理完数据后，继续接收数据
                    }

                } catch (const std::exception& e) {
                    std::cout << e.what() << std::endl;
                }
            },
            boost::asio::detached);
}

void CSession::Close() {
    _socket.close();
    _b_closed = true;
    // remove session from service
    _service->ClearSession(_sessionId);
}

void CSession::Send(const std::string& msg, short msgId) { Send(msg.c_str(), msg.length(), msgId); }

void CSession::Send(const char* msg, short len, short msgId) {
    std::unique_lock<std::mutex> lock(_send_mutex);
    int que_size = _send_que.size();
    if (que_size > MAX_SENDQUE) {
        std::cout << "send queue is full" << std::endl;
        return;
    }

    _send_que.push(std::make_shared<SendNode>(msg, len, msgId));


    //  如果当前发送队列为空，则调用发送函数，如果已经有消息在发送，则直接返回（消息会链式调用发送完）
    if (que_size > 1) {
        return;
    }

    auto msgNode = _send_que.front();
    lock.unlock();
    boost::asio::async_write(
            _socket, boost::asio::buffer(msgNode->_data, msgNode->_total_len),
            [this, msgNode](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                // 注意传递this指针的方式
                this->HandleSend(ec, bytes_transferred, shared_from_this());
                // _send_que.pop();
            });
}

void CSession::HandleSend(const boost::system::error_code& ec, std::size_t bytes_transferred,
                          std::shared_ptr<CSession> self) {
    try {
        if (!ec) {
            std::unique_lock<std::mutex> lock(_send_mutex);
            _send_que.pop();
            if (!_send_que.empty()) {
                auto msgNode = _send_que.front();
                lock.unlock();
                boost::asio::async_write(
                        _socket, boost::asio::buffer(msgNode->_data, msgNode->_total_len),
                        [this, msgNode](const boost::system::error_code& ec,
                                        std::size_t bytes_transferred) {
                            this->HandleSend(ec, bytes_transferred, shared_from_this());
                        });
            }

        } else {
            std::cout << "send error: " << ec.message() << std::endl;
            Close();
            _service->ClearSession(_sessionId);
        }

    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
