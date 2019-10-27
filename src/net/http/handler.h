#ifndef LILY_SRC_NET_HTTP_HANDLER_H_
#define LILY_SRC_NET_HTTP_HANDLER_H_
#include "../../common/log.h"
#include <functional>
namespace lily {
  namespace http {
    using Handler = std::function<void(const Request &req, Response &rsp)>;
    inline void NotFound(const Request &req, Response &rsp) {
      rsp.status_code = StatusNotFound;
    }
    inline void Redirect(const Request &req, Response &rsp, const std::string &where) {
      rsp.status_code = StatusTemporaryRedirect;
      rsp.header.Add("Location", where);
    }
  }
}
#endif
