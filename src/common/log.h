#ifndef LILY_SRC_COMMON_LOG_H_
#define LILY_SRC_COMMON_LOG_H_
#include "../coroutine/co_mutex.h"
#include "../interface/io.h"
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
//#define CONTEXT __FILE__ ":" STR(__LINE__)
#define CONTEXT nullptr
//#define FUNCTION __func__
#define FUNCTION nullptr

  struct IterableBuf : public std::stringbuf {
    char *begin() { return this->pbase(); }
    char *end() { return this->pptr(); }
  };

  class LogStream {
    const Writer &m_logger;
    std::ostringstream m_os;
   public:
    LogStream(const Writer &logger,
              const char *context,
              const char *function,
              bool print_time = false,
              std::string_view prefix = "")
        : m_logger(logger) {
      if (context != nullptr) {
        m_os << context;
      }
      if (function != nullptr) {
        m_os << context << "[" << function << "]";
      }
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

  inline Writer DefaultLogWriter = lily::Stdout;
  inline Writer DefaultLogErrorWriter = lily::Stderr;

#define LogDebug lily::LogStream(DefaultLogWriter, CONTEXT, FUNCTION, true, "[DEBUG]")
#define LogError lily::LogStream(DefaultLogErrorWriter, CONTEXT, FUNCTION, true, "[ERROR]")
#define LogInfo lily::LogStream(DefaultLogWriter, CONTEXT, FUNCTION, true, "[INFO] ")
}

#endif
