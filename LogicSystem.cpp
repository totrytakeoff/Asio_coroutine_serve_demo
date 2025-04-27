#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <iostream>
#include "LogicSystem.h"

// 严重程度：HIGH
// 问题描述：消息投递没有大小限制，可能导致内存溢出
// 优化建议：添加消息队列大小限制，实现消息丢弃策略
void LogicSystem::PostMsgtToQue(std::shared_ptr<LogicNode> msg) {
    std::unique_lock<std::mutex> lock(_msgQueMutex);

    _msgQue.push(msg);

    // 唤醒工作线程
    // 如果消息队列中只有一个消息，则通知工作线程
    if (_msgQue.size() == 1) {
        _msgQueCondVar.notify_all();
    }
}

// 严重程度：MEDIUM
// 问题描述：消息回调注册没有错误处理
// 优化建议：添加重复注册检查和错误处理
void LogicSystem::RegisterMsgCallBack() {
    _msgCallBackMap.insert(std::make_pair(
            MSG_HELLOWORLD,
            [this](std::shared_ptr<CSession> self, const short& msg_id,
                   const std::string& msg_data) { HelloWorld(self, msg_id, msg_data); }));
}

// 严重程度：CRITICAL
// 问题描述：构造函数中直接启动线程，不符合RAII原则
// 优化建议：使用二级构造模式，将线程启动逻辑移到单独的Start()方法中
LogicSystem::LogicSystem() : _is_running(true) {
    // 注册消息回调函数
    RegisterMsgCallBack();
    // 启动工作线程
    _worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

// 严重程度：CRITICAL
// 问题描述：消息处理过程中持有锁，可能导致死锁
// 优化建议：使用临时队列，减少锁的持有时间
void LogicSystem::DealMsg() {
    while (_is_running) {
        std::unique_lock<std::mutex> lock(_msgQueMutex);

        while (_msgQue.empty() && _is_running) {
            // 等待消息队列不为空
            _msgQueCondVar.wait(lock);
        }

        if (!_is_running) {
            while (!_msgQue.empty()) {
                auto msg = _msgQue.front();
                std::cout << " --recv the msg::" << msg->_recvNode->_data
                          << "--- msg id :" << msg->_session->GetSessionId() << std::endl;
                if (_msgCallBackMap.find(msg->_recvNode->_msg_id) != _msgCallBackMap.end()) {
                    _msgCallBackMap[msg->_recvNode->_msg_id](msg->_session, msg->_recvNode->_msg_id,
                                                             msg->_recvNode->_data);
                }
                _msgQue.pop();
            }

            break;
        }


        // 处理消息
        while (!_msgQue.empty()) {
            auto msg = _msgQue.front();
            std::cout << " --recv the msg::" << msg->_recvNode->_data
                      << "--- msg id :" << msg->_session->GetSessionId() << std::endl;
            if (_msgCallBackMap.find(msg->_recvNode->_msg_id) != _msgCallBackMap.end()) {
                _msgCallBackMap[msg->_recvNode->_msg_id](msg->_session, msg->_recvNode->_msg_id,
                                                         msg->_recvNode->_data);
            }
            _msgQue.pop();
        }
    }
}

// 严重程度：HIGH
// 问题描述：JSON解析没有完善的错误处理
// 优化建议：添加详细的错误检查和日志
void LogicSystem::HelloWorld(std::shared_ptr<CSession> session, const short& msg_id,
                             const std::string& msg_data) {
    Json::Value jsonData;
    Json::Reader reader;
    if (reader.parse(msg_data, jsonData)) {
        std::string name = jsonData["name"].asString();


        std::cout << "HelloWorld!, I am " << name << "!" << std::endl;
        jsonData["name"] = "Server";
        std::string ret= jsonData.toStyledString();
        
        session->Send(ret, MSG_HELLOWORLD);
    } else {
        std::cerr << "Invalid JSON data." << std::endl;
    }

}

// 严重程度：HIGH
// 问题描述：析构函数可能无法正确清理所有资源
// 优化建议：确保所有消息都被处理完，添加超时机制
LogicSystem::~LogicSystem() {
    _is_running = false;
    _msgQueCondVar.notify_all();
    _worker_thread.join();
}