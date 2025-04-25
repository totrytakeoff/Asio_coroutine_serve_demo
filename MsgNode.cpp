#include "MsgNode.h"

MsgNode::MsgNode(short max_len) : _cur_len(0), _total_len(max_len) {
    _data = new char[max_len + 1];
    _data[max_len] = '\0';
}
MsgNode::~MsgNode() { delete[] _data; }

void MsgNode::ClearNode() {
    _cur_len = 0;
    // _total_len = 0;
    memset(_data, 0, sizeof(_data));
}


RecvNode::RecvNode(short max_len, short msg_id) : MsgNode(max_len), _msg_id(msg_id) {}


// 注意发送长度要加上消息头长度
SendNode::SendNode(const char* msg, short max_len, short msg_id)
        : MsgNode(max_len + HEADER_LENGTH), _msg_id(msg_id) {
    // 先发送ID ，转化为网络字节序
    short net_id = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
    memcpy(_data, &net_id, HEADER_ID_LENGTH);
    // 再发送数据长度
    short net_len = boost::asio::detail::socket_ops::host_to_network_short(max_len);
    memcpy(_data + HEADER_ID_LENGTH, &net_len, HEADER_DATA_LENGTH);

    // 最后发送数据：

}
