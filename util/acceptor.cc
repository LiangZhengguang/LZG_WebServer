#include "acceptor.h"
#include "address.h"
#include "channel.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
using namespace my_muduo;


const int kMaxListenNum = 5;

Acceptor::Acceptor(EventLoop* loop, Address& address)
    :acceptloop_(loop),
     acceptfd_(::socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)),
     idlefd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
     acceptchannel_(new Channel(loop, acceptfd_)){
      SetSockoptReuseAddr(acceptfd_);
      SetSockoptKeepAlive(acceptfd_);
      accept_bind(address);
      acceptchannel_->SetReadCallback(std::bind(&Acceptor::NewConn, this));
}

Acceptor::~Acceptor() {
  acceptchannel_->DisableAll();
  acceptloop_->Remove(acceptchannel_.get());
  ::close(acceptfd_);
}


void Acceptor::accept_bind(Address& addr){
  struct sockaddr_in address;
  bzero((char*)&address, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(addr.port());
  int ret = bind(acceptfd_, (struct sockaddr*)(&address), sizeof(address));
  (void)ret;
  assert(ret != -1); 
}

void Acceptor::accept_listen(){
    int ret = listen(acceptfd_, kMaxListenNum);
    assert(ret != -1);
    (void)ret;
    printf("accept_listen\n");
    acceptchannel_->EnableRead();
    
}


void Acceptor::NewConn(){
    struct sockaddr_in client, peeraddr;
    socklen_t client_addrlength = sizeof( client );
    int connfd = ::accept4(acceptfd_, (struct sockaddr*)&client, &client_addrlength, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0) {
      if (errno == EMFILE) {
        ::close(idlefd_);
        idlefd_ = ::accept(acceptfd_, NULL, NULL);
        ::close(idlefd_);
        idlefd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
      }
      return;
    }
    assert(connfd > 0);
    if (SetSockoptKeepAlive(connfd) == -1) {
      LOG_ERROR << "Acceptor::NewConnection SetSockoptKeepAlive failed";
      close(connfd);
      return;
    }
    if (SetSockoptTcpNoDelay(connfd) == -1) {
    LOG_ERROR << "Acceptor::NewConnection SetSockoptTcpNoDelay failed";
      close(connfd);
      return;
    }
    socklen_t peer_addrlength = sizeof(peeraddr);
    getpeername(connfd, (struct sockaddr *)&peeraddr, &peer_addrlength);
    newconn_callback_(connfd, Address(inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port)));
}