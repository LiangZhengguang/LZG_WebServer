#include"buffer.h"
#include <sys/uio.h>

using namespace my_muduo;

int Buffer::ReadFd(int fd){
    char extrabuf[65536];
    int writeable_size = writeable();
    iovec vec[2];
    vec[0].iov_base = begin_write();
    vec[0].iov_len = writeable_size;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writeable_size < static_cast<int>(sizeof(extrabuf))) ? 2 : 1;
    const int n = static_cast<int>(::readv(fd, vec, iovcnt)); // 通过栈上的缓存都一次读进来

    if(n < 0){
        // LOG_ERRNO << "Buffer::ReadFd readv failed";
    }
    else if(n <= writeable_size)
        write_index_ += n;
    else{ // 在用户空间把栈上的缓存中的加入到buffer中  无需再使用系统调用读出来
        write_index_ = static_cast<int>(buffer_.size());
        Append(extrabuf, n - writeable_size);
    }
    return n;
}