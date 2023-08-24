#ifndef MY_MUDUO_TIMER_H_
#define MY_MUDUO_TIMER_H_
#include <functional>
#include "noncopyable.h"
#include "timestamp.h"
namespace my_muduo {

class Timer : public NonCopyAble {
public:
    typedef std::function<void()> BasicFunc;
    Timer(Timestamp timestamp, BasicFunc&& cb, double interval);
    void Restart(Timestamp now) {expiration_ = Timestamp::AddTime(now, interval_);}
    void Run() const {callback_();}
    
    Timestamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }

private:
    Timestamp expiration_; // 终止时间
    BasicFunc callback_; // 
    double interval_;  // 计时时间
    bool repeat_;  // 是否循环

};

}
#endif