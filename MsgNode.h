#ifndef MSGNODE_H
#define MSGNODE_H


#include <boost/asio.hpp>
#include <string>
#include "const.h"


#pragma once

class MsgNode {
public:
    MsgNode(short max_len);

    ~MsgNode();

    void ClearNode();
// protected:
    short _cur_len;
    short _total_len;
    char* _data;
};


class RecvNode : public MsgNode {
public:
    RecvNode(short max_len, short msg_id);
    short _msg_id;
};


class SendNode : public MsgNode {
public:
    SendNode(const char* msg, short max_len, short msg_id);
    short _msg_id;
};



#endif