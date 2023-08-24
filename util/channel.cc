#include"channel.h"


using namespace my_muduo;


Channel::Channel(EventLoop* loop, int& fd):
    loop_(loop),
    fd_(fd),
    event_(0),
    revent_(0),
    state_(kNew){
}

void Channel::HandleEvent() {
  if (revent_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    if(read_callback_) {
      read_callback_();
    }
  } 

  if (revent_ & EPOLLOUT) {
    if(write_callback_) {
      write_callback_();
    }
  }
}


