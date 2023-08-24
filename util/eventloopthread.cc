#include"eventloopthread.h"

using namespace my_muduo;

EventLoopThread::EventLoopThread():
    loop_(nullptr),
    thread_(std::bind(&EventLoopThread::StartFunc, this)),
    mutex_(),
    cond_(mutex_){

}
// 主线程调用     去启动一个线程执行创建并执行loop()， 这里返回创建的loop_
EventLoop* EventLoopThread::StartLoop(){
    // 子线程去创建loop 
    thread_.Start(); 
    EventLoop* loop;
    {
        MutexGraud lock(mutex_);
        // 等待子线程创建出来
        while(loop_ == nullptr){
            cond_.Wait();
        }
        loop = loop_;
    }
    // 直接返回loop_ 可能存在返回前被其他线程改变loop_的风险
    return loop;
}

// 这里是由thread_线程去执行的
void EventLoopThread::StartFunc(){
    // eventloopthread还没有真正的loop  创建一个 让loop_指向它
    EventLoop loop;
    // 因为创建和启动要同步 先创建再启动 所以要保护loop_不被其他的线程改动
    {
        MutexGraud lock(mutex_);
        loop_ = &loop;    
        cond_.Notify();
    }
    loop.Loop();
    {
        MutexGraud lock(mutex_);
        loop_ = nullptr;    
    }
}