#ifndef MY_MUDUO_CONDITION_H_
#define MY_MUDUO_CONDITION_H_

#include"pthread.h"
#include"mutex.h"

#include <stdint.h>
#include <sys/time.h>
#include <errno.h>

class Condition{
public:
    explicit Condition(Mutex& mutex):mutex_(mutex){pthread_cond_init(&cond_, NULL); }
    ~Condition(){ pthread_cond_destroy(&cond_); }
    void Wait(){ pthread_cond_wait(&cond_, mutex_.Get()); }
    void Notify(){ pthread_cond_signal(&cond_); }
    void notifyAll() { pthread_cond_broadcast(&cond_); }
    
    bool WaitForFewSeconds(double seconds) {
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);

        const int64_t kNanoSecondsPerSecond = 1000000000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

        time.tv_sec += static_cast<time_t>((time.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
        time.tv_nsec = static_cast<long>((time.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

        return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.pthreadmutex(), &time);
    }

private:
    Mutex& mutex_;
    pthread_cond_t cond_;

};

#endif