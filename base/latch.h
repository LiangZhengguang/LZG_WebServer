#ifndef MY_MUDUO_LATCH_H_
#define MY_MUDUO_LATCH_H_
#include"mutex.h"
#include"condition.h"
/*
封装了一个条件变量和互斥锁  实现计数
*/

namespace my_muduo{

class Latch{
public:
    explicit Latch(int count):count_(count), mutex_(), cond_(mutex_){}
    void CountDown(){
        MutexGraud mutexlock(mutex_);// 改变count 先上锁
        count_--;
        if(count_ == 0) cond_.notifyAll();
    }

    void Wait(){
        MutexGraud mutexlock(mutex_); 
        while(count_ > 0)
            cond_.Wait(); // 进入wait先解锁   等待notify   wait后会再加锁
        // 执行完毕 MutexGraud析构 自动unlock   mutex_
    }

private:
    int count_;
    Mutex mutex_;
    Condition cond_; 
};

}

#endif
