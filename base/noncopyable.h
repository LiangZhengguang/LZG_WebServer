#ifndef MY_MUDUO_NONCOPYABLE_H_
#define MY_MUDUO_NONCOPYABLE_H_

namespace my_muduo {

class NonCopyAble {
 protected:
  NonCopyAble() {}
  ~NonCopyAble() {}
 
 private:
  NonCopyAble(const NonCopyAble&) = delete;
  NonCopyAble& operator=(const NonCopyAble&) = delete;
};

}

#endif
