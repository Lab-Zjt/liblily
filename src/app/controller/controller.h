#ifndef LILY_SRC_APP_CONTROLLER_CONTROLLER_H_
#define LILY_SRC_APP_CONTROLLER_CONTROLLER_H_
#include "../../common/common.h"
#include "../../net/net.h"
#include "../proto/proto.h"
class Controller {
 private:
  const char *m_path;
  Ref<lily::TCPServer> m_server;
  Ref<lily::Client> m_conn;
  proto::ProtoHandler m_handler;
 public:
  Controller(const char *path) : m_path(path) {
    Error err;
    tie(m_server, err) = lily::TCPServer::ListenTCP(m_path);
    if (err != NoError) {
      std::cerr << "Listen TCP failed.\n";
      return;
    }
  }
  Error StartNewServer(const std::string &ip, uint16_t port, const std::string& root) {
    if (system("./http_server &") < 0) {
      perror("http_server");
      return Error{"create sub process failed"};
    }
    std::thread([this, ip = ip, port, root = root]() -> Error {
      auto[conn, err] = m_server->Accept();
      if (err != NoError && conn == nullptr) {
        return err;
      }
      m_conn = conn;
      proto::StartServerNotify notify;
      notify.addr = ip;
      notify.port = port;
      notify.root = root;
      err = proto::SendNotify(conn, notify);
      if (err != NoError) {
        return err;
      }
      while (true) {
        auto err = m_handler.Handle(conn);
        if (err != NoError) {
          return NoError;
        }
      }
    }).detach();
    return NoError;
  }
  Error StopServer() {
    proto::StopServerNotify notify;
    notify.reason = "Stop by command";
    if (auto err = proto::SendNotify(m_conn, notify); err != NoError) {
      return err;
    } else {
      m_conn = nullptr;
      return NoError;
    }
  }
  template<typename Notify, typename Fn>
  void RegisterHandler(Fn &&fn) {
    m_handler.RegisterHandler<Notify>(std::forward<Fn>(fn));
  }
};

#endif //LILY_SRC_APP_CONTROLLER_CONTROLLER_H_
