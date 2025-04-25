#include "CService.h"
#include <iostream>


CService::CService(boost::asio::io_context& ioc, short port)
        : _ioc(ioc)
        , _port(port)
        , _acceptor(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
    StartAccept();
}

CService::~CService() {}

void CService::ClearSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(_mutex);
    _sessions.erase(sessionId);
}

void CService::StartAccept() {
    // 从IOServicePool中获取一 IOcontext,用于处理连接
    auto& ioc = IOServicePool::Instance().GetIOService();
    std::shared_ptr<CSession> session = std::make_shared<CSession>(ioc, this);
    _acceptor.async_accept(
            session->GetSocket(),
            [this, session](const boost::system::error_code& ec) { HandleAccept(session, ec); });
}

void CService::HandleAccept(std::shared_ptr<CSession> session,
                            const boost::system::error_code& ec) {
    if (!ec) {
        // 连接成功，添加到会话列表
        session->Start();
        std::lock_guard<std::mutex> lock(_mutex);
        _sessions.insert(std::make_pair(session->GetSessionId(), session));

    } else {
        std::cout << "Accept error: " << ec.message() << std::endl;
    }

    // 继续接受下一个连接
    StartAccept();

}
