#ifndef LILY_SRC_NET_HTTP_HEADER_H_
#define LILY_SRC_NET_HTTP_HEADER_H_

#include <unordered_map>

namespace lily {
  namespace http {
    class Header {
      std::unordered_map<std::string, std::string> m_header;
     public:
      void Add(const std::string &key, const std::string &val) {
        if (auto it = m_header.find(key); it != m_header.end()) {
          it->second += "; " + val;
        } else {
          m_header[key] = val;
        }
      }
      void Del(const std::string &key) {
        m_header.erase(key);
      }
      void Mod(const std::string &key, const std::string &val) {
        m_header[key] = val;
      }
      decltype(auto) Get() const {
        return m_header;
      }
    };
  }
}

#endif
