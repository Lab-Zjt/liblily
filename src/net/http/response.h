#ifndef LILY_SRC_NET_HTTP_RESPONSE_H_
#define LILY_SRC_NET_HTTP_RESPONSE_H_

#include "header.h"
#include "status_code.h"
#include "../../interface/io.h"
#include "../../common/slice.h"

namespace lily {
  namespace http {
    struct Response {
     public:
      std::string version;
      StatusCode status_code = StatusOK;
      Header header;
     private:
      Writer m_writer;
      bool m_header_wrote = false;
      bool m_chunked = false;
      bool m_no_body = false;
     public:
      Response(Writer wr) : m_writer(std::move(wr)) {}
      ~Response() {
        if (!m_header_wrote && !m_chunked) {
          m_no_body = true;
          WriteHeader();
        } else if (m_chunked) {
          char buf[] = "0\r\n\r\n";
          Write(span(buf, 5));
        }
      }
      R<ssize_t, Error> WriteHeader() {
        m_header_wrote = true;
        auto str = "HTTP/" + version + " " + std::to_string(status_code) + StatusDesc[status_code] + "\r\n";
        bool has_content_length = false;
        for (auto &&h : header.Get()) {
          if (h.first == "Content-Length") {
            has_content_length = true;
          }
          str += h.first + ": " + h.second + "\r\n";
        }
        if (!has_content_length && !m_no_body) {
          str += "Transfer-Encoding: chunked\r\n";
          m_chunked = true;
        }
        str += "\r\n";
        auto[c1, err1] = m_writer.Write(span(str));
        if (err1 != NoError) {
          return {c1, err1};
        }
        return {c1, NoError};
      }
      R<ssize_t, Error> Write(span<char> buf) {
        ssize_t all = 0;
        if (!m_header_wrote) {
          header.Add("Content-Length", std::to_string(buf.size()));
          auto[c1, err1] = WriteHeader();
          if (err1 != NoError) {
            return {c1, err1};
          }
          while (all < buf.size()) {
            std::tie(c1, err1) = m_writer.Write(buf.sub(all, buf.size()));
            if (err1 != NoError) {
              return {all, err1};
            }
            all += c1;
          }
          return {all, NoError};
        }
        // write chunk size
        char size_str[32];
        auto c = sprintf(size_str, "%lx", buf.size());
        size_str[c] = '\r';
        size_str[c + 1] = '\n';
        auto[c2, err] = m_writer.Write(span(size_str, c + 2));
        if (err != NoError) {
          return {all, err};
        }
        //all += c2;
        // write buf
        while (all < buf.size()) {
          std::tie(c2, err) = m_writer.Write(buf.sub(all, buf.size()));
          if (err != NoError) {
            return {all, err};
          }
          all += c2;
        }
        return {all, NoError};
      }
    };
  }
}

#endif //LILY_SRC_NET_HTTP_RESPONSE_H_
