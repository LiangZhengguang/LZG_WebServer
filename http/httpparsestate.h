#ifndef MY_MUDUO_HTTPSTATE_H_
#define MY_MUDUO_HTTPSTATE_H_

namespace my_muduo {

enum HttpRequestParseState {
  kParseRequestLine,
  kParseHeaders,
  kParseBody,
  kParseGotCompleteRequest,
  kParseErrno,
};

}

#endif
