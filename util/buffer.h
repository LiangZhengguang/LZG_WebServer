#ifndef MY_MUDUO_BUFFER_
#define MY_MUDUO_BUFFER_

#include<vector>
#include<string>
#include<assert.h>
#include<algorithm>
#include<string.h>
#include "noncopyable.h"
using std::string;

namespace my_muduo{

static const int pre_index_ = 8;
static const int kInitialSize = 1024;
static const char* kCRLF = "\r\n";

class Buffer : public NonCopyAble {
public:
    Buffer():
        buffer_(kInitialSize),
        read_index_(pre_index_),
        write_index_(pre_index_){
    }
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); };

    char* begin_read() { return begin() + read_index_; } 
    const char* begin_read() const { return begin() + read_index_; } 

    char* begin_write() { return begin() + write_index_; } 
    const char* begin_write() const{ return begin() + write_index_; } 

    const char* FindCRLF() const { 
        const char* find = std::search(Peek(), begin_write(), kCRLF, kCRLF + 2); 
        return find == begin_write() ? nullptr : find;
    }

    const char* Peek() const {return begin_read();}
    char* Peek() {return begin_read();}

    string PeekAsString(int len) {return string(begin_read(), begin_read() + len);}
    string PeekAllAsString() {return string(begin_read(), begin_write()); }


    int readable() const { return write_index_ - read_index_; } //pre -- read
    int writeable() const { return static_cast<int>(buffer_.size()) - write_index_; } // read -- write
    int preable() const { return read_index_; } // 0 -- pre_ind
    
    void MakeEnough(int len){
        if(writeable() > len) return;
        if(writeable() + preable() >= pre_index_ + len){
            std::copy(begin_read(), begin_write(), begin() + pre_index_);
            write_index_ = readable() + pre_index_;
            read_index_ = pre_index_;
        }
        else{
           buffer_.resize(buffer_.size() + len); 
        }
    }

    void Append(const char* message) {Append(message, static_cast<int>(strlen(message)));}
    void Append(const string& message){Append(&*message.begin(), static_cast<int>(message.length()));}
    void Append(const char* message, int len) {
        MakeEnough(len);
        std::copy(message, message + len, begin_write());
        write_index_ += len;
    }

    void Retrieve(int len){
        // assert(readablebytes() >= len);
        if(len + read_index_ < write_index_){
            read_index_ += len;
        }
        else{
            RetrieveAll();
        }
    }

    void RetrieveAll() { // 重置索引
        write_index_ = pre_index_;
        read_index_ = write_index_;
    }

    void RetrieveUntilIndex(const char* index) {
        assert(begin_write() >= index);
        read_index_ += static_cast<int>(index - begin_read());
    } 

    string RetrieveAsString(int len) {
        assert(read_index_ + len <= write_index_);
        string ret = std::move(PeekAsString(len));
        Retrieve(len); 
        return ret;
    }

    string RetrieveAllAsString() {
        string ret = std::move(PeekAllAsString());
        RetrieveAll();
        return ret;
    }



    int ReadFd(int fd);

private:
    std::vector<char> buffer_;
    // int pre_index_;
    int read_index_;
    int write_index_;
    
};

}

#endif