#include "server.h"
#include "../../coroutine/go.h"
#include "../../common/log.h"

namespace lily {
  namespace http {
    void Server::Serve() {
      while (true) {
        if (m_stop) {
          LogDebug << "stopping server...";
          break;
        }
        // Endless Loop
        auto[conn, err1] = m_server.Accept();
        if (err1 != NoError) {
          LogError << "accept error. " << err1.desc;
        }
        if (conn == nullptr) {
          if (err1 == EAGAIN || err1 == EBADF)break;
          else continue;
        }
        LogInfo << "connection from " << conn->PeerAddr().String() << " to " << conn->LocalAddr().String();
        go([this, conn = std::move(conn)]() {
          Request req(conn);
          Response rsp(conn);
          auto err = ParseHTTPHeader(req, BufIO::Reader(conn));
          rsp.version = req.version;
          if (err != NoError) {
            LogError << "parse HTTP header failed. " << err.desc;
            NotFound(req, rsp);
          } else {
            m_handler(req, rsp);
          }
        });
      }
    }
  }
}