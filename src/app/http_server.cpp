#include <fstream>
#include "../net/http/server.h"
#include "../coroutine/go.h"

using namespace lily;

std::optional<std::string> read_all(const std::string &filename) {
  std::ifstream fs(filename);
  if (!fs) {
    return std::nullopt;
  }
  return std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>{});
}

void handler(const http::Request &req, http::Response &rsp) {
  if (req.path == "/task_count") {
    auto count = std::to_string(lily::Dispatcher::Get().GetTaskCount());
    LogInfo << "task count is " << count;
    rsp.Write(span(count));
    return;
  }
  auto str = read_all("." + req.path);
  if (str) {
    if (auto[c, err] = rsp.Write(span(*str)); err != NoError) {
      LogError << "write back failed. " << err.desc;
    } else {
      LogDebug << "write " << c << " bytes";
    }
  } else {
    http::NotFound(req, rsp);
  }
  LogInfo << req.method << " " << req.path << " " << rsp.status_code;
}

uint16_t random_select_port() {
  size_t port = 0;
  srand((unsigned) time(nullptr));
  while (port < 1024) { port = rand() % 65536; }
  return port;
}

Main(int argc, char *argv[]) {
  auto port = 12345;
  http::Server server("127.0.0.1", port, handler);
  LogInfo << "listen at 127.0.0.1:" << port;
  server.Serve();
  return 0;
}