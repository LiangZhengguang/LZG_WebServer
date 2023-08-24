#ifndef MY_MUDUO_ACCEPTOR_H_
#define My_MUDUO_ACCEPTOR_H_
#include <functional>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "noncopyable.h"
/*
掌管bind listen accept 有了新链接时创建connfd，传给回调上层的newconn(int connfd)
*/
namespace my_muduo{

class EventLoop;
class Channel;
class Address;

class Acceptor : public NonCopyAble{
public:
    typedef std::function<void (int, const Address&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, Address& address);
    ~Acceptor();
    int SetSockoptKeepAlive(int fd) {
        int option_val = 1;
        return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &option_val, static_cast<socklen_t>(sizeof(option_val)));
    }

    int SetSockoptReuseAddr(int fd) {
        int option_val = 1;
        return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                        &option_val, static_cast<socklen_t>(sizeof(option_val)));
    }

    int SetSockoptTcpNoDelay(int fd) {
        int option_val = 1;
        return setsockopt(fd, IPPROTO_TCP, SO_KEEPALIVE,
                        &option_val, static_cast<socklen_t>(sizeof(option_val)));
        
    }

    // 绑定  在acceptor初始化是就绑定  属于acceptor的工作
    void accept_bind(Address& address);
    // 开启监听  由tcpserver控制打开
    void accept_listen();
    // accept() 接受新的conn  并执行用户定义的链接函数
    void NewConn();
    void SetConnCallback(const NewConnectionCallback& newconn){newconn_callback_ = newconn;};
    void SetConnCallback(NewConnectionCallback&& newconn){newconn_callback_ = std::move(newconn);};
private:
    EventLoop* acceptloop_;
    int acceptfd_;
    int idlefd_;
    std::unique_ptr<Channel> acceptchannel_;
    NewConnectionCallback newconn_callback_;
};

}

#endif