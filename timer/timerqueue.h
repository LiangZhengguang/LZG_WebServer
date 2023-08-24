#ifndef MY_MUDUO_TIMERQUEUE_H_
#define MY_MUDUO_TIMERQUEUE_H_
#include <unistd.h>
#include <set>
#include <vector>
#include <memory>
#include "timer.h"
#include "timestamp.h"
#include "noncopyable.h"
#include "logging.h"
namespace my_muduo{
class EventLoop;
class Channel;
class TimerQueue : public NonCopyAble {
public:
    typedef std::function<void()> BasicFunc;
    typedef std::pair<Timestamp, Timer*> TimerPair;
    typedef std::set<TimerPair> TimersSet; 
    typedef std::vector<TimerPair> ActiveTimers;

    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    void ReadTimerFd(){
        uint64_t read_byte;
        ssize_t readn = ::read(timerfd_, &read_byte, sizeof(read_byte));
        if (readn != sizeof(read_byte)) {
            LOG_ERROR	 << "TimerQueue::ReadTimerFd read_size < 0";
        }
    }

    // 当定时器触发时，loop接受到时间，channel执行timerqueue设置好的回调HandleRead
    void HandleRead() {
        ReadTimerFd(); // 读出来---虽然不使用读出来的东西，但防止下次再触发
        // 
        Timestamp expiration_time(Timestamp::Now()); 
        active_timers_.clear(); 
        // 以当前时间做一个TimerPair，找到timers_第一个大于当前时间的end
        auto end = timers_.lower_bound(TimerPair(Timestamp::Now(), reinterpret_cast<Timer*>(UINTPTR_MAX)));
        // 将小于当前时间的放入激活timer  应该是考虑到可能多个timer同时到达
        active_timers_.insert(active_timers_.end() , timers_.begin(), end);
        // 删掉已经到时间的timer
        timers_.erase(timers_.begin(), end);
        // 执行计时器的callback_
        for (const auto& timerpair : active_timers_) {
            timerpair.second->Run();
        } 
        ResetTimers();
    }

    void ResetTimers() {
        for (auto& timerpair: active_timers_) {
            if ((timerpair.second)->repeat()) {
                auto timer = timerpair.second;
                timer->Restart(Timestamp::Now());
                Insert(timer);
            } else {
                delete timerpair.second;
            }
        }
        if (!timers_.empty()) {
            ResetTimer(timers_.begin()->second);
        }
    }

    bool Insert(Timer* timer) {
        bool reset_instantly = false;
        // pair集合空 || timer的终止时间小于timers第一个计数器的时间 这时候要修改计数器触发时间
        if (timers_.empty() || timer->expiration() < timers_.begin()->first ) {
            // 标记重置这个计数器
            reset_instantly = true;
        }
        timers_.emplace(std::move(TimerPair(timer->expiration(), timer)));
        return reset_instantly;
    }

    void AddTimerInLoop(Timer* timer) {
        // printf("AddTimerInLoop......\n");
        bool reset_instantly = Insert(timer);
        if (reset_instantly) {
            ResetTimer(timer);
        }
    }

    void ResetTimer(Timer* timer);
    void AddTimer(Timestamp timestamp, BasicFunc&& cb, double interval);

private:
    EventLoop* loop_;
    int timerfd_;
    std::unique_ptr<Channel> channel_; 
    
    TimersSet timers_;
    ActiveTimers active_timers_;
};

}
#endif