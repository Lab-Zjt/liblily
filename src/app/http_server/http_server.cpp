#include <fstream>
#include "../../net/http/server.h"
#include "../../net/http/mime.h"
#include "../../coroutine/go.h"
#include "../../common/log.h"
#include "../../app/proto/proto.h"

using namespace lily;

std::optional<std::string> read_all(const std::string &filename) {
  std::ifstream fs(filename);
  if (!fs) {
    return std::nullopt;
  }
  return std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>{});
}

void countReqTimes(const http::Request &req, http::Response &rsp) {
  auto req_count = 0;
  if (auto it = req.header.Get().find("Cookie"); it != req.header.Get().end()) {
    auto pos = it->second.find("req_count=");
    if (pos == std::string::npos) {
      req_count = 0;
    } else {
      req_count = atoi(it->second.substr(pos + 10).c_str());
    }
  }
  rsp.header.Add("Set-Cookie", "req_count=" + std::to_string(req_count + 1));
}

void handler(const http::Request &req, http::Response &rsp, const std::string &root) {
  if (req.path == "/task_count") {
    auto count = std::to_string(lily::Dispatcher::Get().GetTaskCount());
    LogInfo << "task count is " << count;
    rsp.Write(span(count));
    return;
  }
  // avoid hacker :(
  if (req.path.find("/../") != std::string::npos) {
    http::NotFound(req, rsp);
  } else {
    auto str = read_all(root + req.path);
    if (str) {
      rsp.header.Add("Content-Type", get_mime(req.path));
      if (auto[c, err] = rsp.Write(span(*str)); err != NoError) {
        LogError << "write back failed. " << err.desc;
      } else {
        LogDebug << "write " << c << " bytes";
      }
    } else {
      http::NotFound(req, rsp);
    }
  }
  LogInfo << req.method << " " << req.path << " " << rsp.status_code;
}

Main(int argc, char *argv[]) {
  proto::ProtoHandler handle;
  auto[client, err] = Client::Connect(NetProtocol::TCP, "/tmp/http_server_controller");
  if (err != NoError) {
    std::cerr << "Connect to controller failed. " << err.desc << "\n";
    return -1;
  }
  auto w = New<proto::LogWriter>(client);
  DefaultLogWriter = w;
  DefaultLogErrorWriter = w;
  handle.RegisterHandler<proto::StartServerNotify>(
      [client = client](const proto::StartServerNotify &notify) mutable {
        go([notify, client = std::move(client)]() mutable {
          auto[server, err] = http::Server::Listen(
              notify.addr.c_str(),
              notify.port,
              [notify](const http::Request &req, http::Response &rsp) {
                handler(req, rsp, notify.root);
              });
          if (err != NoError) {
            LogError << "Listen at " << notify.addr << ":" << notify.port << " failed.";
            proto::StartServerFailedNotify n;
            n.reason = err.desc;
            proto::SendNotify(client, n);
            exit_all();
            return;
          }
          LogInfo << "Listen at " << notify.addr << ":" << std::to_string(notify.port) << " Work Directory: "
                  << notify.root;
          server->Serve();
        });
        return NoError;
      });
  handle.RegisterHandler<proto::StopServerNotify>(
      [](const proto::StopServerNotify &notify) mutable {
        exit_all();
        return NoError;
      });
  while (true) {
    auto err = handle.Handle(client);
    if (err == EAGAIN || err == EBADF) {
      break;
    }
  }
  return 0;
}