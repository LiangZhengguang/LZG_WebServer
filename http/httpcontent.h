#ifndef MY_MUDUO_HTTPCONTENT_H_
#define MY_MUDUO_HTTPCONTENT_H_

#include <utility>
#include <algorithm>

#include "buffer.h"
#include "httprequest.h"
#include "httpparsestate.h"

namespace my_muduo {

enum HttpRequestParseLine {
  kLineOK,
  kLineMore,
  kLineErrno
};

class HttpContent {
 public:
  HttpContent();
  ~HttpContent();

  void ParseLine(Buffer* buffer);
  bool ParseContent(Buffer* buffer);
  bool GetCompleteRequest() { return parse_state_ == kParseGotCompleteRequest; } 
  
  const HttpRequest& request() const { return request_; }
  void ResetContentState() { 
    HttpRequest tmp;
    request_.Swap(tmp);
    parse_state_ = kParseRequestLine;
  }

 private:
  HttpRequest request_;
  HttpRequestParseState parse_state_;

};

}
#endif
