#ifndef LILY_SRC_NET_HTTP_HANDLER_H_
#define LILY_SRC_NET_HTTP_HANDLER_H_
#include <functional>
namespace lily {
  namespace http {
    using Handler = std::function<void(const Request &req, Response &rsp)>;
    inline void NotFound(const Request &req, Response &rsp) {
      static std::string warning_msg("404 Page Not Found");
      rsp.status_code = StatusNotFound;
      rsp.header.Add("Content-Length", std::to_string(warning_msg.size()));
      rsp.Write(span(warning_msg));
    }
    inline void Redirect(const Request &req, Response &rsp, const std::string &where) {
      rsp.status_code = StatusTemporaryRedirect;
      rsp.header.Add("Location", where);
    }
  }
}
#endif
