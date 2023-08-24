#include"eventloop.h"
#include "channel.h"

using namespace my_muduo;

EventLoop::EventLoop()
    : tid_(::pthread_self()),
      epoller_(new Epoller()),
      wakeup_fd_(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      calling_functors_(false),
      timer_queue_(new TimerQueue(this)){
    wakeup_channel_->SetReadCallback(std::bind(&EventLoop::HandleRead, this));
    wakeup_channel_->EnableRead();
}

EventLoop::~EventLoop(){
    wakeup_channel_->DisableAll();
    Remove(wakeup_channel_.get());
    close(wakeup_fd_);
}

void EventLoop::Loop(){
    while(1){
        ActiveChannel_.clear();
        epoller_->FillActiveChannels(ActiveChannel_);
        for (const auto& channel : ActiveChannel_) {
            // printf("From Loop connfd %d :",  channel->fd());
            channel->HandleEvent();
        }
        DoToDoList();
    }
}

void EventLoop::HandleRead(){
    uint64_t read_one_byte = 1;
    ssize_t read_size = ::read(wakeup_fd_, &read_one_byte, sizeof(read_one_byte));
    (void) read_size;
    assert(read_size == sizeof(read_one_byte));
    return;
    return;
}


void EventLoop::QueueOneFunc(BaseFunc func) {
  {  
    MutexGraud lock(mutex_);
    to_do_list_.emplace_back(std::move(func));
  }

  if (!IsInThreadLoop() || calling_functors_) {
    uint64_t write_one_byte = 1;  
    ssize_t write_size = ::write(wakeup_fd_, &write_one_byte, sizeof(write_one_byte));
    (void) write_size;
    assert(write_size == sizeof(write_one_byte));

  } 
}

void EventLoop::RunOneFunc(BaseFunc func){
    // 其他线程可能需要eventloop去执行 例如主线程将connection加到其他线程loop
    // printf("RunOneFunc...\n");
    if(IsInThreadLoop()){
        // printf("func...\n");
        func();
    }
    else{
        // printf("QueueOneFunc...\n");
        QueueOneFunc(std::move(func));
    }
}


void EventLoop::DoToDoList(){
    ToDoList funclists;
    calling_functors_ = true;
    {   
        MutexGraud lock(mutex_);
        to_do_list_.swap(funclists);
    }
    for(auto func : funclists)
        func();
    calling_functors_ = false;
}