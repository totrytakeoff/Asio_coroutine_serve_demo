#ifndef CONST_H
#define CONST_H

#define MAX_LENGTH 1024*2 //最大消息长度

#define HEADER_LENGTH 4 //消息头长度
#define HEADER_ID_LENGTH 2 //消息ID长度
#define HEADER_DATA_LENGTH 2 //数据长度
#define MAX_RECVQUE 10000 //接收队列最大长度
#define MAX_SENDQUE 1000 //发送队列最大长度


enum MsgID{
    MSG_HELLOWORLD = 1001,

};



#endif