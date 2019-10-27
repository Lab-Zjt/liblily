#ifndef LILY_SRC_COMMON_LOG_H_
#define LILY_SRC_COMMON_LOG_H_
#include "../coroutine/co_mutex.h"
#include <sstream>
#include <unistd.h>
namespace lily {

  struct FileWriter {
    CoroutineMutex m_mtx;
    int fd;
   public:
    FileWriter(int fd_) : fd(fd_) {}
    R<ssize_t, Error> Write(span<char> buf) {
      std::lock_guard<CoroutineMutex> lock(m_mtx);
      auto c = write(fd, buf.data(), buf.size());
      if (c < 0) { return {c, ERRNO}; }
      return {c, NoError};
    }
  };

  inline Writer Stdout = New<FileWriter>(1);
  inline Writer Stderr = New<FileWriter>(2);

#define TOSTR(x) #x
#define STR(x) TOSTR(x)
#define CONTEXT __FILE__ ":" STR(__LINE__)

  struct IterableBuf : public std::stringbuf {
    char *begin() { return this->pbase(); }
    char *end() { return this->pptr(); }
  };

  class LogStream {
    Writer &m_logger;
    std::ostringstream m_os;
   public:
    LogStream(Writer &logger,
              const char *context,
              const char *function,
              bool print_time = false,
              std::string_view prefix = "")
        : m_logger(logger) {
      m_os << context << "[" << function << "]";
      m_os << prefix;
      if (print_time) {
        auto tm = std::time(nullptr);
        char time_buf[64];
        auto c = std::strftime(time_buf, 64, "[%F %T]", std::localtime(&tm));
        m_os.write(time_buf, c);
      }
    }
    ~LogStream() {
      m_os << "\n";
      auto it = reinterpret_cast<IterableBuf *>(m_os.rdbuf());
      m_logger.Write(span<char>(it->begin(), it->end() - it->begin()));
    }
    template<typename T>
    LogStream &operator<<(T &&t) {
      m_os << std::forward<T>(t);
      return *this;
    }
  };

#define LogDebug lily::LogStream(lily::Stdout, CONTEXT, __func__, false, "[DEBUG]")
#define LogError lily::LogStream(lily::Stderr, CONTEXT, __func__, true, "[ERROR]")
#define LogInfo lily::LogStream(lily::Stdout, CONTEXT, __func__, true, "[INFO] ")
}

#endif
