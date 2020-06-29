#include "http.h"
#include "../../common/log.h"

namespace lily {
  namespace http {
    Error ParseRequestFirstLine(Request &req, const std::string &str) {
      size_t off = 0;
      auto e1 = str.find(' ', off);
      if (e1 == std::string::npos) {
        return Error{"parse method error"};
      }
      req.method = str.substr(off, e1 - off);
      off = e1 + 1;
      auto e2 = str.find(' ', off);
      if (e2 == std::string::npos) {
        return Error{"parse path error"};
      }
      req.raw_path = str.substr(off, e2 - off);
      req.url = URL::Parse(req.raw_path);
      off = e2 + 1;
      auto e3 = str.find('/', off);
      if (e3 == std::string::npos) {
        return Error{"parse version error"};
      }
      req.version = str.substr(e3 + 1);
      return NoError;
    }
    Error ParseResponseFirstLine(Response &rsp, const std::string &str) {
      size_t off = 0;
      auto e1 = str.find(' ', off);
      if (e1 == std::string::npos || e1 < sizeof("HTTP/x.x")) {
        return Error{"parse version error"};
      }
      rsp.version = str.substr(e1 - 3, 3);
      off = e1 + 1;
      auto e2 = str.find(' ', off);
      if (e2 == std::string::npos) {
        return Error{"parse status code error"};
      }
      rsp.status_code = static_cast<StatusCode>(atoi(str.substr(off, e2 - e1).c_str()));
      return NoError;
    }
    Error ParseHTTPHeader(Request &req, lily::BufIO::Reader reader) {
      auto[line, err1] = reader.ReadLine();
      if (err1 == EndOfFile) {
        LogError << "unexpected EOF.";
        return err1;
      }
      if (err1 != NoError) {
        LogError << "readline error. " << err1.desc;
        return err1;
      }
      err1 = ParseRequestFirstLine(req, line);
      if (err1 != NoError) {
        LogError << "parse HTTP header error. " << line;
        return err1;
      }
      while (true) {
        std::tie(line, err1) = reader.ReadLine();
        if (err1 == EndOfFile) {
          LogError << "unexpected EOF.";
          return err1;
        }
        if (err1 != NoError) {
          LogError << "readline error. " << err1.desc;
          return err1;
        }
        if (line.empty()) {
          break;
        }
        err1 = ParseHeaderField(req, line);
        if (err1 != NoError) {
          LogError << "parse header field failed.";
          return err1;
        }
      }
      return NoError;
    }

    template<typename HeaderField>
    Error ParseHeaderFiledImpl(HeaderField &h, const std::string &str) {
      auto c1 = str.find(": ");
      if (c1 == std::string::npos) {
        return Error{"parse header field failed"};
      }
      h.header.Add(str.substr(0, c1), str.substr(c1 + 2));
      return NoError;
    }

    Error ParseHeaderField(Request &h, const std::string &str) {
      return ParseHeaderFiledImpl(h, str);
    }
    Error ParseHeaderField(Response &h, const std::string &str) {
      return ParseHeaderFiledImpl(h, str);
    }
  }
}