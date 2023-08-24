#ifndef MY_MUDUO_TCPSERVER_H_
#define MY_MUDUO_TCPSERVER_H_
#include "callback.h"
#include "eventloop.h"
#include "acceptor.h"
#include "eventloopthreadpool.h"



namespace my_muduo{
/*
TcpServer 拥有一个Acceptor来接收

*/
class Address;

class TcpServer{
public:
    TcpServer(EventLoop* loop, Address& address);
    ~TcpServer();
    // 由用户设置回调，
    void set_message_callback(const MessageCallback& mc){message_callback_ = mc;}
    void set_message_callback(MessageCallback&& mc){message_callback_ = std::move(mc);}
    void set_connect_callback(const ConnectionCallback& cc){connect_callback_ = cc;}
    void set_connect_callback(ConnectionCallback&& cc){connect_callback_ = std::move(cc);}


    void HandleClose(const std::shared_ptr<TcpConnection>& conn);
    void HandleCloseInLoop(const std::shared_ptr<TcpConnection>& ptr);
    void HandleNewConn(int connfd, const Address& address);

    void Start(){
        thread_pool_->StartLoop(); 
        base_loop_->RunOneFunc(std::bind(&Acceptor::accept_listen, acceptor_));
    }
    void SetThreadNums(int thread_nums){ thread_pool_->SetThreadNum(thread_nums); }
private:
    typedef std::unordered_map<int, std::shared_ptr<TcpConnection>> ConnectionMap;

    EventLoop* base_loop_;
    EventLoopThreadPool* thread_pool_;
    Acceptor* acceptor_;
    
    MessageCallback message_callback_;
    ConnectionCallback connect_callback_;

    ConnectionMap connections_;
    int next_connection_id_;
    const std::string ipport_;
};

}
#endif