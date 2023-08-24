#include"epoller.h"
#include"channel.h"
#include <sys/epoll.h>
#include<fcntl.h>
#include <string.h>
#include <assert.h>
using namespace my_muduo;

Epoller::Epoller():
    epollfd_(epoll_create(MAX_EVENTS)),
    events_(MAX_EVENTS),
    channelmap_(){
}

Epoller::~Epoller(){
    ::close(epollfd_);
}

void Epoller::FillActiveChannels(Channels& active_channels){
    int eventnums = Wait();
    for(int i = 0; i < eventnums; i++){
        Channel* channel_ptr = (Channel*)events_[i].data.ptr; // 得到事件对应的channel指针
        channel_ptr->set_revent(events_[i].events);
        active_channels.emplace_back(channel_ptr); // 把活跃的channel加入到loop的active_channels
    }
    if (eventnums == static_cast<int>(events_.size())) {
        events_.resize(eventnums * 2);
    }
}

void Epoller::Update(Channel* channel){
    int operation = 0;
    int cur_event = channel->event();
    ChannelState state = channel->state(); 
    int fd = channel->fd();

    if (state == kNew || state == kDeleted) {
        if(state == kNew){
            assert(channelmap_.find(fd) == channelmap_.end());
            channelmap_[fd] = channel;
        }
        else{
            assert(channelmap_.find(fd) != channelmap_.end());
            assert(channelmap_[fd] == channel);
        }
        operation = EPOLL_CTL_ADD;
        channel->SetChannelState(kAdded);
    } 
    else {
        assert(channelmap_.find(fd) != channelmap_.end());
        assert(channelmap_[fd] == channel);
        if (cur_event == 0) {
            operation = EPOLL_CTL_DEL;
            channel->SetChannelState(kDeleted); 
        } else {
            operation = EPOLL_CTL_MOD; 
        }
    }
    // 在epoll里面加入channel的fd 来监听事件
    UpdateChannel(channel, operation);
}

void Epoller::UpdateChannel(Channel* channel , int operation){
    epoll_event event;
    // memset(&event, '\0', sizeof(struct epoll_event));
    event.events = channel->event();
    event.data.ptr = static_cast<void*>(channel);
    epoll_ctl(epollfd_, operation, channel->fd(), &event);
}

void Epoller::Remove(Channel* channel){
    int fd = channel->fd();
    ChannelState state = channel->state();
    assert(state == kDeleted || state == kAdded);
    if (state == kAdded) {
        UpdateChannel(channel, EPOLL_CTL_DEL);
    }
    channel->SetChannelState(kNew);
    channelmap_.erase(fd);
}   