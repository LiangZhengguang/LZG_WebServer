#ifndef MY_MUDUO_TCPCONNECTIONPTR_H_
#define My_MUDUO_TCPCONNECTIONPTR_H_
/*
TcpConnection是对Channel的封装(个人认为) 属于某一个loop
负责管理conn的发送 接受
*/

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <utility>
#include <memory>

#include "buffer.h"
#include "callback.h"
#include "channel.h"
#include "httpcontent.h"
#include "noncopyable.h"
#include "timestamp.h"
namespace my_muduo{

class Channel;
class EventLoop;


class TcpConnection: public std::enable_shared_from_this<TcpConnection>, NonCopyAble {
public:
    enum ConnectionState {
        kConnected,
        kDisconnected,
        kDisconnecting
    };

    TcpConnection(EventLoop* loop,int connfd, int id);
    ~TcpConnection();

    void set_massage_callback(const MessageCallback& callback){message_callback_ = callback; }
    void set_massage_callback(MessageCallback&& callback){message_callback_ = std::move(callback); }

    void set_connect_callback(const ConnectionCallback& callback){connect_callback_ = callback; }
    void set_connect_callback(ConnectionCallback&& callback){connect_callback_ = std::move(callback); }

    void set_close_callback(const CloseCallback& callback){close_callback_ = callback;}
    void set_close_callback(CloseCallback&& callback){close_callback_ = std::move(callback);}
    
    void ConnectionEstablished(){
        // printf("ConnectionEstablished tid: %ld", ::pthread_self);
        state_ = kConnected;
        connchannel_->EnableRead();
        connect_callback_(shared_from_this(), &input_buffer_);
    }
    
    void ConnectionDestructor();

    void HandleError();
    void HandleClose();
    void HandleMessage();
    void HandleWrite();

    void Send(Buffer* buffer);
    void Send(const string& str);
    void Send(const char* message, int len);
    void Send(const char* message) { Send(message, static_cast<int>(strlen(message))); }

    int GetErrno() const;
    void Shutdown();
    bool IsShutdown() const {return  state_ == kDisconnecting; } 

    int fd(){return connfd_;}
    int id() const { return id_; }
    EventLoop* loop(){return loop_;}
    HttpContent* GetHttpContent() {return &content_;}
    Timestamp timestamp(){return timestamp_;} // 上次活跃的时间
    void UpdateTimestamp(Timestamp time){timestamp_ = time;}
private:
    EventLoop* loop_;
    int id_;
    int connfd_;
    bool shutdown_;
    ConnectionState state_;
    std::unique_ptr<Channel> connchannel_;
    Timestamp timestamp_;

    Buffer input_buffer_;
    Buffer output_buffer_;

    HttpContent content_;
    
    MessageCallback message_callback_;
    ConnectionCallback connect_callback_;
    CloseCallback close_callback_ ;

    typedef std::shared_ptr<TcpConnection> TcpconnectionPtr;

};

}
#endif