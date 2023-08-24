#include"eventloopthreadpool.h"

using namespace my_muduo;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop):
    thread_num_(0),
    base_loop_(loop),
    loops_(),
    threads_(),
    next_(0){
}

void EventLoopThreadPool::StartLoop(){
    
    for(int i = 0; i < thread_num_; i++){
        EventLoopThread* ptr = new EventLoopThread();
        threads_.emplace_back(ptr);
        loops_.emplace_back(ptr->StartLoop());
    }
}

EventLoop* EventLoopThreadPool::NextLoop(){
    EventLoop* ret = base_loop_;
    if(!loops_.empty()){
        ret = loops_[next_];
        next_ = (next_ + 1) % thread_num_;
    }
    return ret;
}