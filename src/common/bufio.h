#ifndef LILY_SRC_COMMON_BUFIO_H_
#define LILY_SRC_COMMON_BUFIO_H_
#include "slice.h"
#include "../interface/io.h"
#include <string>
namespace lily {
  namespace BufIO {
    class Reader {
      array<char> m_buf;
      ::Reader rd;
      size_t r, w;
     public:
      Reader(const ::Reader &rd_) : m_buf(4096), rd(rd_), r(0), w(0) {}
      R<ssize_t, Error> Read(span<char> buf) {
        if (buf.empty()) return {0, NoError};
        if (r == w) {
          // 缓冲区为空，一次性读取超过缓冲区容量的字节，直接读进buf里避免copy
          if (buf.size() > m_buf.size()) {
            return rd.Read(buf);
          }
          r = w = 0;
          auto[n1, err1] = rd.Read(buf);
          if (err1 != NoError) {
            return {n1, err1};
          }
          w += n1;
        }
        auto n = w - r;
        memcpy(buf.data(), m_buf.data() + r, n);
        r += n;
        return {n, NoError};
      }
      R<std::string, Error> ReadLine() {
        std::string line;
        // 表示起点
        auto n = r;
        // 寻找\n
        for (;;) {
          if (r == w) {
            // 先将缓冲区中的字符写入line
            line.append(m_buf.as_chars() + n, r - n);
            // 清空缓冲区
            n = w = r = 0;
            // 从reader中读取字符进缓冲区
            auto[n1, err1] = rd.Read(m_buf.as_span());
            // 出现错误时返回
            if (err1 != NoError) {
              return {line, err1};
            }
            if (n1 == 0) {
              return {line, EndOfFile};
            }
            // 设置缓冲区的已读为n1
            w = n1;
          }
          // 如果找到，此时m_buf[r]为\n
          if (m_buf[r] == '\n') {
            // 如果开头就是\n
            if (n == r) {
              // 如果line中已经有内容，且最后一个字符是\r，说明换行符是\r\n，去掉line中的\r
              if (!line.empty() && line.back() == '\r') { line.pop_back(); }
            } else {
              // 如果换行符是\r\n，则line需要去掉前面的\r
              size_t drop = m_buf[r - 1] == '\r' ? 1 : 0;
              line.append(m_buf.as_chars() + n, r - n - drop);
            }
            r++;
            return {line, NoError};
          }
          r++;
          // 到达缓冲区末尾依然没找到
        }
      }
    };
  }
}


#endif
