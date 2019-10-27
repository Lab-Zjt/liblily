#ifndef LILY_SRC_NET_HTTP_H_
#define LILY_SRC_NET_HTTP_H_
#include "request.h"
#include "response.h"
#include "../../common/bufio.h"
namespace lily {
  namespace http {

    Error ParseRequestFirstLine(Request &req, const std::string &str);
    Error ParseResponseFirstLine(Response &rsp, const std::string &str);
    Error ParseHTTPHeader(Request &req, BufIO::Reader reader);
    Error ParseHeaderField(Request &h, const std::string &str);
    Error ParseHeaderField(Response &h, const std::string &str);
  }
}

#endif //LILY_SRC_NET_HTTP_H_
