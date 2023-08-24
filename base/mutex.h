#ifndef MY_MUDUO_MUTEX_H_
#define MY_MUDUO_MUTEX_H_
#include"pthread.h"

class Mutex{
public:
    Mutex() {
        pthread_mutex_init(&mutex_,NULL);
    }

    ~Mutex(){
        // pthread_mutex_lock(&mutex_);// linya多了这一行  这一保证没有人持有锁的时候销毁锁
        pthread_mutex_destroy(&mutex_);
    }
    pthread_mutex_t* pthreadmutex() {return &mutex_;}
    void Lock(){pthread_mutex_lock(&mutex_);}
    void Unlock(){pthread_mutex_unlock(&mutex_);}
    pthread_mutex_t* Get(){return &mutex_;}
private:
    pthread_mutex_t  mutex_;
};

class MutexGraud{
public:
    explicit MutexGraud(Mutex& mutex) : mutex_(mutex) {
        mutex_.Lock();
    }
    // 局部一个MutexGraud时 可以自动的析构并解锁
    ~MutexGraud(){ mutex_.Unlock(); }
private:
    Mutex& mutex_;
};

#endif