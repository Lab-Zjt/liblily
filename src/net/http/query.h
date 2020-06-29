#ifndef LILY_SRC_NET_HTTP_QUERY_H_
#define LILY_SRC_NET_HTTP_QUERY_H_

namespace lily {
  namespace http {
    using Query = std::unordered_map<std::string, std::string>;
    inline Query ParseQuery(const std::string &raw) {
      Query query;
      size_t off = 0;
      while (off < raw.size()) {
        auto key_begin = off;
        auto sep = raw.find('&', off);
        auto key_end = raw.find('=', off);
        if (key_end == std::string::npos) break;
        auto val_begin = key_end + 1;
        query[raw.substr(key_begin, key_end - key_begin)] = raw.substr(val_begin, sep - val_begin);
        if (sep == std::string::npos)break;
        off = sep + 1;
      }
      return query;
    }
  }
}

#endif
