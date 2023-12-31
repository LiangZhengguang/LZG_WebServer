#ifndef MY_MUDUO_LOGGING_H_
#define MY_MUDUO_LOGGING_H_

#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

#include <cstring>

#include "timestamp.h"
#include "logstream.h"
#include "currentthread.h"
#include "noncopyable.h"

namespace my_muduo {
                                                                                
class SourceClass {
 public:
  SourceClass(const char* data) : data_(data), len_(static_cast<int>(strlen(data_))) {
    const char* forward_slash = strrchr(data, '/');
    if (forward_slash) {
      data_ = forward_slash + 1;
      len_ -= static_cast<int>((data_ - data)); 
    } 
  }  
  
  const char* data_;
  int len_; 
};

class Logger : public NonCopyAble {
 public:
  enum Level {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
  };

  Logger(const char* file_, int line, Level level) 
      : implement_(file_, line, level) {
  }

  LogStream& stream() { return implement_.stream(); }
   
  typedef void (*OutputFunc)(const char* data, int len);
  typedef void (*FlushFunc)();

 private:
  class Implement : public NonCopyAble {
   public:
    typedef Logger::Level Level;

    Implement(SourceClass&& source, int line, Level level); 
    ~Implement();

    void FormattedTime();
    const char* GetLogLevel() const;

    void Finish() { stream_ << " - "
                    << GeneralTemplate(fileinfo_.data_, fileinfo_.len_) 
                    << ':' << line_ << '\n'; 
    }
 
    LogStream& stream() { return stream_; }

   private:
    SourceClass fileinfo_;
    int line_; 
    Level level_;
   
    LogStream stream_;
 };

  Implement implement_;
};

} //namespace my_muduo

my_muduo::Logger::Level LogLevel();
void SetLogLevel(my_muduo::Logger::Level nowlevel);
const char* ErrorToString(int err);

#define LOG_DEBUG if (LogLevel() <= my_muduo::Logger::DEBUG) \
                    my_muduo::Logger(__FILE__, __LINE__, my_muduo::Logger::DEBUG).stream()
#define LOG_INFO  if (LogLevel() <= my_muduo::Logger::INFO) \
					my_muduo::Logger(__FILE__, __LINE__, my_muduo::Logger::INFO).stream()
#define LOG_WARN my_muduo::Logger(__FILE__, __LINE__, my_muduo::Logger::WARN).stream()
#define LOG_ERROR my_muduo::Logger(__FILE__, __LINE__, my_muduo::Logger::ERROR).stream()
#define LOG_FATAL my_muduo::Logger(__FILE__, __LINE__, my_muduo::Logger::FATAL).stream()

#endif