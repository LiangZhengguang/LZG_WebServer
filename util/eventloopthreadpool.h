#ifndef MY_MUDUO_EVENTLOOPTHREADLOOP_H_
#define MY_MUDUO_EVENTLOOPTHREADLOOP_H_

#include"eventloop.h"
#include"eventloopthread.h"

namespace my_muduo{

class EventLoopThreadPool{
public:
    typedef std::vector<EventLoopThread*> Threads;
    typedef std::vector<EventLoop*> Loops;

    EventLoopThreadPool(EventLoop* loop); 
    ~EventLoopThreadPool(){};
    void SetThreadNum(int n){thread_num_ = n;}
    void StartLoop();
    EventLoop* NextLoop();
private:
    int thread_num_;
    EventLoop* base_loop_; //主loop
    Loops loops_;
    Threads threads_;
    int next_;
};

}
#endif