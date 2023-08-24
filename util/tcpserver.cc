#include"tcpserver.h"
#include"tcpconnection.h"
#include <assert.h>
#include <limits.h>
#include"address.h"
using namespace my_muduo;


TcpServer::TcpServer(EventLoop* loop, Address& address)
    : base_loop_(loop),
      thread_pool_(new EventLoopThreadPool(loop)),
      acceptor_(new Acceptor(loop, address)),
      next_connection_id_(1),
      ipport_(std::move(address.IpPortToString())){
        acceptor_->SetConnCallback(std::bind(&TcpServer::HandleNewConn, this, _1, _2));
        printf("server construct\n");
}

TcpServer::~TcpServer(){
    for (auto& pair : connections_) {
        std::shared_ptr<TcpConnection> ptr(pair.second);
        pair.second.reset();
        ptr->loop()->RunOneFunc(std::bind(&TcpConnection::ConnectionDestructor, ptr));
  }
}


void TcpServer::HandleClose(const std::shared_ptr<TcpConnection>& ptr) {
    base_loop_->QueueOneFunc(std::bind(&TcpServer::HandleCloseInLoop, this, ptr));
}

void TcpServer::HandleCloseInLoop(const std::shared_ptr<TcpConnection>& ptr) {
    assert(connections_.find(ptr->fd()) != connections_.end());
    connections_.erase(connections_.find(ptr->fd()));
    EventLoop* loop = ptr->loop(); 
    loop->QueueOneFunc(std::bind(&TcpConnection::ConnectionDestructor, ptr));
}

// 收到新连接 有acceptor回调这个函数， 创建tcpconnection
void TcpServer::HandleNewConn(int connfd, const Address& address){
    EventLoop* loop = thread_pool_->NextLoop();
    // printf("New Conn to Thread(loop): %d\n", loop->GetTid());
    std::shared_ptr<TcpConnection> ptr(new TcpConnection(loop, connfd, next_connection_id_));
    connections_[connfd] = ptr;
    ptr->set_connect_callback(connect_callback_);
    ptr->set_massage_callback(message_callback_);
    ptr->set_close_callback(std::bind(&TcpServer::HandleClose, this, _1));
    // printf("loopthread: %ld, currthread: %ld ",loop->GetTid(), ::pthread_self);
    LOG_INFO << "TcpServer::HandleNewConnection - new connection " << "[" 
           << ipport_ << '#' << next_connection_id_ << ']' << " from " 
           << address.IpPortToString();
    next_connection_id_++;
    if (next_connection_id_ == INT_MAX) next_connection_id_ = 1;
    loop->RunOneFunc (std::bind(&TcpConnection::ConnectionEstablished, ptr)); // 新来的连接绑定到loop上
    // printf("currthread: %ld \n", ::pthread_self);
}

