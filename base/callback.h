#ifndef MY_MUDUO_CALLBACK_
#define MY_MUDUO_CALLBACK_
#include <functional>
#include "buffer.h"
#include <utility>
#include <memory>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace my_muduo {
  class TcpConnection;
  class Buffer;
  typedef std::function<void (const std::shared_ptr<TcpConnection>&, Buffer*)> ConnectionCallback;
  typedef std::function<void (const std::shared_ptr<TcpConnection>&, Buffer*)> MessageCallback;
  typedef std::function<void ()> ReadCallback;
  typedef std::function<void ()> WriteCallback;
  typedef std::function<void (const std::shared_ptr<TcpConnection>&)> CloseCallback;  
}

#endif