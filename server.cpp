#include "src/coroutine/go.h"
#include "src/net/net.h"
#include "src/coroutine/co_chan.h"
#include "src/coroutine/co_select.h"
#include <iostream>
#include <sys/eventfd.h>
#include <sys/epoll.h>
template<typename T>
using Ptr = std::unique_ptr<T>;

template<typename T>
using Arr = std::unique_ptr<T[]>;

template<typename T>
Arr<T> NewArr(size_t t) {
  return std::unique_ptr<T[]>(new T[t]);
}

using namespace lily;

#define _case(var, chan, ...) CreateCase(chan, [&](typename decltype(chan)::value_type var){__VA_ARGS__})

Main(int argc, char **argv) {
  Channel<int> ci;
  Channel<double> cd;
  Channel<std::string> cs;
  go([&]() {
    auto s = Select(
        _case(i, ci,
              std::cout << "int: " << i << std::endl;
        ),
        _case(d, cd,
              std::cout << "double: " << d << std::endl;
        ),
        _case(s, cs,
              std::cout << "string: " << s << std::endl;
        ),
        _case(_, _default,
              std::cout << "default: " << std::endl;
        )
    );
    for (;;) {
      s.Select();
    }
  });
  TCPServer server("127.0.0.1", 12998);
  int index = 0;
  for (;;) {
    auto conn = server.Accept();
    if (conn == nullptr) {
      std::cerr << "accept failed\n";
      continue;
    }
    printf("%s <=> %s\n", conn->PeerAddr().String().c_str(), conn->LocalAddr().String().c_str());
    if (++index % 3 == 0) { ci << index; }
    else if (index % 3 == 1) { cd << index; }
    else if (index % 3 == 2) { cs << std::to_string(index); }
    go([](std::unique_ptr<Client> conn) {
      auto buf = NewArr<char>(1024);
      auto len = conn->Read(buf.get(), 32);
      buf[len] = '\0';
      conn->Write(buf.get(), len);
      std::cout << "char[" << len << "]: " << buf.get() << "\n";
    }, std::move(conn));
  }
  return 0;
}