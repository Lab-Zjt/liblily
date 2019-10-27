#ifndef LILY_SRC_NET_HTTP_REQUEST_H_
#define LILY_SRC_NET_HTTP_REQUEST_H_

#include "../../interface/io.h"
#include "header.h"

namespace lily {
  namespace http {
    struct Request {
      std::string method;
      std::string path;
      std::string version;
      Header header;
      Reader body;
      Request(Reader rd) : body(std::move(rd)) {}
    };
  }
}

#endif //LILY_SRC_NET_HTTP_REQUEST_H_
