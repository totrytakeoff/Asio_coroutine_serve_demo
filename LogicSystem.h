#ifndef LOGICSYSTEM_H
#define LOGICSYSTEM_H

#pragma once

#include <queue>
#include <thread>
#include <map>
#include <string>
#include <functional>
#include "const.h"
#include "CSession.h"


using FuncCallBack = std::function<void(std::shared_ptr<CSession>,const short& msg_id,const std::string & msg_data)>;

class LogicSystem
{
public:
    ~LogicSystem();
    LogicSystem(const LogicSystem&) = delete;
    LogicSystem& operator=(const LogicSystem&) = delete;
    LogicSystem(LogicSystem&&) = delete;

    void PostMsgtToQue(std::shared_ptr<LogicNode> msg);

    void RegisterMsgCallBack();


    static LogicSystem& GetInstance(){
        static LogicSystem instance;
        return instance;
    }

private:
    LogicSystem();
    void DealMsg();
    void HelloWorld(std::shared_ptr<CSession> session,const short& msg_id,const std::string & msg_data);


    std::queue<std::shared_ptr<LogicNode>> _msgQue;
    std::mutex _msgQueMutex;
    std::thread _worker_thread;
    std::condition_variable _msgQueCondVar;

    std::map< short, FuncCallBack> _msgCallBackMap;
    bool _is_running;

};

#endif