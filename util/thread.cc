#include"thread.h"
#include <unistd.h>
#include <sys/syscall.h>

using namespace my_muduo;
namespace CurrentThread {

__thread int t_cachedTid = 0;
__thread char t_formattedTid[32];
__thread int t_formattedTidLength;

pid_t gettid() {
    return static_cast<int>(syscall(SYS_gettid));
}

void CacheTid() {
  if (t_cachedTid == 0) {
    t_cachedTid = gettid();
    t_formattedTidLength = snprintf(t_formattedTid, sizeof(t_formattedTid), "%5d ", t_cachedTid);
  }
}

} // namespace CurrentThread;

static void* ThreadRun(void* arg) {
  ThreadData* ptr = static_cast<ThreadData*>(arg);
  ptr->RunOneFunc();
  delete ptr;
  return nullptr;
}


Thread::Thread(ThreadFunc func)
    :func_(func),
     pthread_id_(-1),
     latch_(1){
}

Thread::~Thread() {
  ::pthread_detach(pthread_id_);
}

void Thread::Start(){
    ThreadData* ptr = new ThreadData(func_, &latch_);
    ::pthread_create(&pthread_id_, NULL, ThreadRun, ptr); // 创建的线程去执行ThreadRun
    latch_.Wait(); // 由于count_是1，会陷入wait 需要等待创建的线程执行CountDown，此时创建完成了，主线程才能继续往下走
}
