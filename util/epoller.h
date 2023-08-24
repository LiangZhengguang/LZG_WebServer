#ifndef MY_MUDUO_EPOLLER_H_
#define MY_MUDUO_EPOLLER_H_
/*
epoller会记录需要关注的channel
*/
#include<sys/epoll.h>
#include<vector>
#include<memory>
#include<unordered_map>

const int MAX_EVENTS  = 8;

namespace my_muduo{
class Channel;

class Epoller{
public:
    typedef std::vector<epoll_event> Events;
    typedef std::vector<Channel*> Channels;
    

    Epoller();
    ~Epoller();
    // void Poll(Channels& channels);
    void FillActiveChannels(Channels& channels);
    int Wait(){return epoll_wait(epollfd_, &*events_.begin(), MAX_EVENTS, -1);};
    void Update(Channel* channel);
    void UpdateChannel(Channel* channel, int operation);
    void Remove(Channel* channel_);

private:
    typedef std::unordered_map<int, Channel*> ChannelMap;
    int epollfd_;
    Events events_;
    ChannelMap channelmap_;
};

}

#endif