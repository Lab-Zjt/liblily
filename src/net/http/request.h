#ifndef LILY_SRC_NET_HTTP_REQUEST_H_
#define LILY_SRC_NET_HTTP_REQUEST_H_

#include "../../interface/io.h"
#include "header.h"
#include "query.h"

namespace lily {
  namespace http {
    struct URL {
      std::string path;
      std::string fragment;
      Query query;
      static URL Parse(const std::string &raw) {
        if (raw.empty())return {};
        URL url;
        int path_begin = 0;
        int path_end = raw.find('?', path_begin);
        url.path = raw.substr(path_begin, path_end - path_begin);
        if (path_end == std::string::npos) {
          return url;
        }
        if (path_end == raw.size() - 1) {
          return url;
        }
        int query_begin = path_end + 1;
        int query_end = raw.find('#', path_begin);
        url.query = ParseQuery(raw.substr(query_begin, query_end - query_begin));
        if (query_end >= raw.size() - 1) {
          return url;
        }
        url.fragment = raw.substr(query_end + 1);
        return url;
      }
    };
    struct Request {
      std::string method;
      std::string raw_path;
      std::string version;
      URL url;
      Header header;
      Reader body;
      Request(Reader rd) : body(std::move(rd)) {}
    };
  }
}

#endif //LILY_SRC_NET_HTTP_REQUEST_H_
