#ifndef MY_MUDUO_EVENTLOOP_H_
#define MY_MUDUO_EVENTLOOP_H_
#include"epoller.h"
#include<functional>
#include <vector>
#include <sys/eventfd.h>
#include <pthread.h>
#include "mutex.h"
#include <unistd.h>
#include <memory>

#include "timestamp.h"
#include "timerqueue.h"
#include "noncopyable.h"

namespace my_muduo{

class Epoller;
class Channel;
class EventLoop{
public:
    typedef std::vector<Channel*> Channels;
    typedef std::function<void()> BaseFunc;
    typedef std::vector<BaseFunc> ToDoList;
    
    // 定时器接口
    void RunAt(Timestamp timestamp, BaseFunc callback){
        timer_queue_->AddTimer(timestamp, std::move(callback), 0.0);
    }

    void RunAfter(double wait_time, BaseFunc cb) {
        Timestamp timestamp(Timestamp::AddTime(Timestamp::Now(), wait_time)); 
        timer_queue_->AddTimer(timestamp, std::move(cb), 0.0);
    }
    
    void RunEvery(double interval, BaseFunc cb) {
        Timestamp timestamp(Timestamp::AddTime(Timestamp::Now(), interval)); 
        timer_queue_->AddTimer(timestamp, std::move(cb), interval);
    }

    EventLoop();
    ~EventLoop();

    void Loop();

    void Update(Channel* channel){epoller_->Update(channel); }
    void Remove(Channel* channel){epoller_->Remove(channel);}
    bool IsInThreadLoop() { return ::pthread_self() == tid_; }

    void HandleRead();
    void DoToDoList();
    void QueueOneFunc(BaseFunc func);
    void RunOneFunc(BaseFunc func);

    pthread_t GetTid() { return tid_; }
private:
    pthread_t tid_;
    std::unique_ptr<Epoller> epoller_; // 每个loop都一个epoller
    int wakeup_fd_; // 唤醒loop
    std::unique_ptr<Channel> wakeup_channel_; 
    bool calling_functors_;
    std::unique_ptr<TimerQueue> timer_queue_;

    Channels ActiveChannel_;  // 用于保存需要处理的channels
    
    ToDoList to_do_list_;  // 待执行的列表

    Mutex mutex_;  // 用于保护 to_do_list_互斥访问

};

}
#endif