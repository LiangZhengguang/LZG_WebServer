#include"tcpconnection.h"
#include "channel.h"
#include <sys/socket.h>
#include <string.h>
using namespace my_muduo;

TcpConnection::TcpConnection(EventLoop* loop, int connfd, int id)
    :loop_(loop),
     id_(id),
     connfd_(connfd),
     shutdown_(false),
     state_(kDisconnected),
     connchannel_(new Channel(loop, connfd)),
     timestamp_(Timestamp::Now()) {
    // channel处理事件 回调用tcpconnect的handle_message
    connchannel_->SetReadCallback(std::bind(&TcpConnection::HandleMessage, this));
    connchannel_->SetWriteCallback(std::bind(&TcpConnection::HandleWrite, this));
}

TcpConnection::~TcpConnection(){ // 断开链接析构时 关闭fd
    ::close(connfd_);
}

int TcpConnection::GetErrno() const {
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);

  if (::getsockopt(connfd_, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

void TcpConnection::ConnectionDestructor(){
    // printf("TcpConnection::ConnectionDestructor\n");
    if(state_ == kDisconnecting || state_ == kConnected){
        state_ = kDisconnected;
        connchannel_->DisableAll();
    }
    loop_->Remove(connchannel_.get());
}

void TcpConnection::HandleError() {
  LOG_ERROR << "TcpConnection::HandleError" << " : " << ErrorToString(GetErrno());
}

void TcpConnection::HandleClose() {
  state_ = kDisconnected;
  connchannel_->DisableAll();
  std::shared_ptr<TcpConnection> guard(shared_from_this());
  close_callback_(guard);
}

void TcpConnection::HandleMessage(){
    // 把收到的消息读出到缓冲区
    int read_size = input_buffer_.ReadFd(connfd_);
    if(read_size > 0)
        message_callback_(shared_from_this(), &input_buffer_);
    else if(read_size == 0){
        HandleClose();
    }
    else {
  	    LOG_ERROR << "TcpConnection::HandleMessage read failed";
    } 
};

void TcpConnection::HandleWrite(){
    if(connchannel_->IsWriting()){
        // 往缓冲区写
        int writeable_size = output_buffer_.readable();
        int remaining = writeable_size;
        int send_size = static_cast<int>(send(connfd_, output_buffer_.begin_read(), remaining, 0));
        if (send_size < 0) {
            assert(send_size > 0);
            if (errno != EWOULDBLOCK) {
                LOG_ERROR << "TcpConnection::HandleWrite write failed";
            }
            return;
        }
        remaining -= send_size;
        // 改索引
        output_buffer_.Retrieve(send_size);

        assert(remaining <= writeable_size);
        // 都读完了就关闭写开关
        if (!output_buffer_.readable()) {
            connchannel_->DisableWrite();
            if (state_ == kDisconnecting) {
                Shutdown();
            }
        }
    }
};

// 用户先将要发送的消息放到output_buffer 当出发可写事件 从output_buffer中发出去
void TcpConnection::Send(Buffer* buffer){  
    // Send(buffer->Peek(), buffer->readablebytes())作者是这样
    Send(buffer->begin_read(), buffer->readable()); 
    buffer->RetrieveAll();
};

void TcpConnection::Send(const string& message){
    Send(message.data(), static_cast<int>(message.size()));
}

void TcpConnection::Send(const char* message, int len){
    int remaining = len;
    int send_size = 0;
    // 如果没有设置可写 或者  没有可以读出来去写的内容
    if (!connchannel_->IsWriting() && output_buffer_.readable() == 0) {
        send_size = static_cast<int>(send(connfd_, message, len, 0));
        if (send_size >= 0){
            remaining -= send_size; 
        }
        else{
            if (errno != EWOULDBLOCK) {
                LOG_ERROR << "TcpConnection::Send write failed";
            }
            return;
        }
    }
    assert(remaining <= len);
    if (remaining > 0) {
        output_buffer_.Append(message + send_size, remaining);
        if (!connchannel_->IsWriting()) { // 因为是LT 所以EnableWrite之后  只要能写就会触发事件
            connchannel_->EnableWrite(); 
        }
    }
}

void TcpConnection::Shutdown() { 
    // printf("TcpConnection::Shutdown");
    state_ = kDisconnecting;
    if (!connchannel_->IsWriting()) {
        int ret = ::shutdown(connfd_, SHUT_WR);
        if (ret < 0) {
            LOG_ERROR << "TcpConnection::Shutdown shutdown failed";
        }
    } 
}