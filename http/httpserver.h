#ifndef MY_MUDUO_HTTPSERVER_H_
#define MY_MUDUO_HTTPSERVER_H_
#include <stdio.h>
#include <functional>
#include <utility>
#include "tcpserver.h"
#include "tcpconnection.h"
#include "buffer.h"
#include "httpcontent.h"
#include "httprequest.h"
#include "httpresponse.h"

using namespace my_muduo;

static const double kIdleConnectionTimeOuts = 8.0;

class HttpServer{

typedef std::function<void (const HttpRequest&, HttpResponse&)> HttpResponseCallback;

public:
    HttpServer(EventLoop* loop, Address& address, bool auto_close_idleconnection = false);
    ~HttpServer(){};
    void Start(){
        server_.Start();
    }
    void HttpDefaultCallback(const HttpRequest& request, HttpResponse& response) {
        response.SetStatusCode(k404NotFound);
        response.SetStatusMessage("Not Found");
        response.SetCloseConnection(true);
        (void)request;
    }

    void HandleIdleConnection(std::weak_ptr<TcpConnection>& connection) {
        // weak_ptr.lock() 会把weak_ptr提升为一个 shared_ptr 对象
        std::shared_ptr<TcpConnection> conn(connection.lock());
        if (conn) { // 上次活跃时间加上8 小于现在的时间  也就是太久没活跃了 主动断开
            if(Timestamp::AddTime(conn->timestamp(), kIdleConnectionTimeOuts)  < Timestamp::Now()) {
                // printf("%s HttpServer::HandleIdleConnection\n", Timestamp::Now().ToFormattedString().data());
                conn->Shutdown();
            } 
            // else { // 8s内活跃过
            //     loop_->RunAfter(kIdleConnectionTimeOuts,std::move(std::bind(&HttpServer::HandleIdleConnection, this, connection)));
            // }
        }
    }

    void ConnectionCallback(const std::shared_ptr<TcpConnection>& connection){
        // printf("%s HttpServer::ConnectionCallback\n", Timestamp::Now().ToFormattedString().data());
        if (auto_close_idleconnection_) { // 自动关闭  开一个8s的计时器 8s后触发HandleIdleConnection 检查是否8s内有活跃
            loop_->RunAfter(kIdleConnectionTimeOuts, std::bind(&HttpServer::HandleIdleConnection, this, std::weak_ptr<TcpConnection>(connection))); 
        }
    }

    void MessageCallback(const std::shared_ptr<TcpConnection>&, Buffer* buffer);

    void SetHttpResponseCallback(const HttpResponseCallback& response_callback) { 
        response_callback_ = response_callback; 
    }
    void SetHttpResponseCallback(HttpResponseCallback&& response_callback) { 
        response_callback_ = std::move(response_callback); 
    } 

    void SetThreadNums(int thread_nums) {server_.SetThreadNums(thread_nums);}

    void DealWithRequest(const HttpRequest& request, const std::shared_ptr<TcpConnection>& connection);

    // 测试计时器用
    // void TestTimer_IntervalEvery3Seconds() const {
    //     printf("%s TestTimer_IntervalEvery3Seconds\n", Timestamp::Now().ToFormattedString().data());
    // }

    // void TestTimer_IntervalEvery5Seconds() const {
    //     printf("%s TestTimer_IntervalEvery5Seconds\n", Timestamp::Now().ToFormattedString().data());
    // }

    // void TestTimer_IntervalEvery10Seconds() const {
    //     printf("%s TestTimer_IntervalEvery10Seconds\n", Timestamp::Now().ToFormattedString().data());
    // }

    // void TestTimer_Interval15SecondsAfterOneLoop() const {
    //     printf("%s TestTimer_Interval15SecondsAfterOneLoop\n", Timestamp::Now().ToFormattedString().data());
    // }

private:
    EventLoop* loop_;
    TcpServer server_;
    bool auto_close_idleconnection_;
    HttpResponseCallback response_callback_;
};

#endif