#ifndef MY_MUDUO_THREAD_H_
#define MY_MUDUO_THREAD_H_

#include"latch.h"
#include <pthread.h>
#include <functional>
#include "noncopyable.h"
namespace my_muduo{
class Thread : public NonCopyAble{
public:
    typedef std::function<void()> ThreadFunc;
    Thread(ThreadFunc func);
    ~Thread();
    void Start();
    // static void* ThreadRun(void* arg);
    void Join() { ::pthread_join(pthread_id_, nullptr); }

private:
    ThreadFunc func_;
    pthread_t pthread_id_;
    Latch latch_;
};

class ThreadData {
public:
    typedef my_muduo::Thread::ThreadFunc ThreadFunc;
    ThreadData(ThreadFunc& func, Latch* latch)
        : func_(func),
        latch_(latch) {
    }
    void RunOneFunc() {
        latch_->CountDown();
        latch_ = nullptr; // 感觉没有必要  ThreadData 后面会delete 每个线程各有一把
        func_();
    }
private:
    ThreadFunc func_;
    Latch* latch_;
};

}
#endif