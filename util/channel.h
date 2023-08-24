#ifndef MY_MUDUO_CHANNEL_H_
#define MY_MUDUO_CHANNEL_H_
#include <sys/epoll.h>
#include "eventloop.h"
#include "callback.h"
#include "noncopyable.h"

namespace my_muduo{

enum ChannelState {
  kNew,
  kAdded, // 表示被添加到epoller
  kDeleted
};

class Channel : public NonCopyAble {
public:
    Channel(EventLoop* loop, int& fd);
    ~Channel(){};
    void HandleEvent();
    void Update(){loop_->Update(this);}
    void EnableRead(){event_ |= EPOLLIN; Update(); }
    void EnableWrite(){event_ |= EPOLLOUT; Update(); }
    void DisableWrite(){event_ &= ~EPOLLOUT; Update(); }
    void DisableAll() {event_ = 0; Update();}

    void SetReadCallback(const ReadCallback& callback) { read_callback_ = callback; }
    void SetReadCallback(ReadCallback&& callback) { read_callback_ = std::move(callback); }
    void SetWriteCallback(const WriteCallback& callback) { write_callback_ = callback;}
    void SetWriteCallback(WriteCallback&& callback) { write_callback_ = std::move(callback);;}

    int fd(){return fd_;}
    int event(){return event_;}
    int revent(){return revent_;}
    ChannelState state(){return state_;}

    bool IsWriting() { return event_ & EPOLLOUT; }
    bool IsReading() { return event_ & EPOLLIN; }

    void set_revent(int event){revent_ = event;}
    // void SetReceivedEvents(int events) {revent_ = events;}
    void SetChannelState(ChannelState state){state_ = state;}
private:
    EventLoop* loop_;

    int fd_;    // 对应的fd 一个channel负责一个fd
    int event_;  // 关注的事件
    int revent_; // 返回的事件

    ReadCallback read_callback_;
    WriteCallback write_callback_;
    ChannelState state_;
};


}


#endif