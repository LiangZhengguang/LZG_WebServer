#ifndef MY_MUDUO_EVENTLOOPTHREAD_H_
#define MY_MUDUO_EVENTLOOPTHREAD_H_
#include"eventloop.h"
#include"thread.h"
#include "condition.h"

namespace my_muduo{

class EventLoopThread{
public:
    EventLoopThread();
    ~EventLoopThread(){};
    void StartFunc();
    EventLoop* StartLoop();
private:
    EventLoop* loop_;
    Thread thread_;
    Mutex mutex_;
    Condition cond_;
};

}
#endif