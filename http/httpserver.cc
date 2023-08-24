#include "httpserver.h"

#include <functional>

using namespace my_muduo;

using my_muduo::Version;

HttpServer::HttpServer(EventLoop* loop, Address& address, bool auto_close_idleconnection)
    : loop_(loop),
      server_(loop, address),
      auto_close_idleconnection_(auto_close_idleconnection) {
    server_.set_connect_callback(std::bind(&HttpServer::ConnectionCallback, this, _1));
    server_.set_message_callback(std::bind(&HttpServer::MessageCallback, this, _1, _2));
    // server_.SetThreadNums(6);
    // 测试计时器用
    // loop_->RunEvery(3.0, std::bind(&HttpServer::TestTimer_IntervalEvery3Seconds, this));
    // loop_->RunEvery(5.0, std::bind(&HttpServer::TestTimer_IntervalEvery5Seconds, this));
    // loop_->RunEvery(10.0, std::bind(&HttpServer::TestTimer_IntervalEvery10Seconds, this));
    // loop_->RunAfter(15.0, std::bind(&HttpServer::TestTimer_Interval15SecondsAfterOneLoop, this));

    SetHttpResponseCallback(std::bind(&HttpServer::HttpDefaultCallback, this, _1, _2));
}

void HttpServer::MessageCallback(const std::shared_ptr<TcpConnection>& connection, Buffer* buffer) {
  
    if (auto_close_idleconnection_) connection->UpdateTimestamp(Timestamp::Now());

    if (connection->IsShutdown()) return;
    
    HttpContent* content = connection->GetHttpContent();
    
    if (!content->ParseContent(buffer)) {
        connection->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
        connection->Shutdown();
    }   
    if (content->GetCompleteRequest()) {
        DealWithRequest(content->request(), connection);
        content->ResetContentState();
    }   
}

void HttpServer::DealWithRequest(const HttpRequest& request, const std::shared_ptr<TcpConnection>& connection) {
  string connection_state = std::move(request.GetHeader("Connection"));
  bool close = (connection_state == "Close" || 
                (request.version() == kHttp10 &&
                connection_state != "Keep-Alive"));

  HttpResponse response(close); 
  response_callback_(request, response);
  Buffer buffer;
  response.AppendToBuffer(&buffer);
  connection->Send(&buffer);

  if (response.CloseConnection()) {
    connection->Shutdown();
  }   
}
