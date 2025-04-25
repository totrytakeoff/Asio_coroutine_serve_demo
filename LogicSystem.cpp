#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <iostream>
#include "LogicSystem.h"
void LogicSystem::PostMsgtToQue(std::shared_ptr<LogicNode> msg) {
    std::unique_lock<std::mutex> lock(_msgQueMutex);

    _msgQue.push(msg);

    // 唤醒工作线程
    // 如果消息队列中只有一个消息，则通知工作线程
    if (_msgQue.size() == 1) {
        _msgQueCondVar.notify_all();
    }
}

void LogicSystem::RegisterMsgCallBack() {
    _msgCallBackMap.insert(std::make_pair(
            MSG_HELLOWORLD,
            [this](std::shared_ptr<CSession> self, const short& msg_id,
                   const std::string& msg_data) { HelloWorld(self, msg_id, msg_data); }));
}


LogicSystem::LogicSystem() : _is_running(true) {
    // 注册消息回调函数
    RegisterMsgCallBack();
    // 启动工作线程
    _worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

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



LogicSystem::~LogicSystem() {
    _is_running = false;
    _msgQueCondVar.notify_all();
    _worker_thread.join();
}