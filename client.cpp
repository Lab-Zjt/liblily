#include "src/coroutine/go.h"
#include "src/net/net.h"
#include "src/coroutine/co_timer.h"
#include <iostream>
template<typename T>
using Ref = std::shared_ptr<T>;


template<typename T>
using Ptr = std::unique_ptr<T>;

using namespace lily;

Main(int argc, char *argv[]) {
  auto ticker = Ticker::New(Second);
  int times = 10;
  while (times-- > 0) {
    auto c = ticker->C();
    std::cout << times << ": " << c << std::endl;
  }
  ticker->Stop = true;
  return 0;
  go([]() {
    Client conn(TCP, "127.0.0.1", 12998);
    char buf[32] = "hello,world";
    auto len = conn.Write(buf, 11);
    std::cout << "write " << len << "\n";
    len = conn.Read(buf, 32);
    std::cout << "read " << len << "\n";
    buf[len] = '\0';
    std::cout << buf << "\n";
  });
  return 0;
}