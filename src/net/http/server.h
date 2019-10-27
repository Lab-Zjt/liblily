#ifndef LILY_SRC_NET_HTTP_SERVER_H_
#define LILY_SRC_NET_HTTP_SERVER_H_

#include "http.h"
#include "handler.h"
#include "../net.h"

namespace lily {
  namespace http {
    class Server {
     private:
      TCPServer m_server;
      Handler m_handler;
      bool m_stop = false;
     public:
      Server(const char *ip, uint16_t port, Handler handler) : m_server(ip, port), m_handler(std::move(handler)) {}
      void Stop() { m_stop = true; }
      void Serve() ;
    };
  }
}

#endif //LILY_SRC_NET_HTTP_SERVER_H_
