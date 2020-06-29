#ifndef LILY_SRC_NET_HTTP_SERVER_H_
#define LILY_SRC_NET_HTTP_SERVER_H_

#include "http.h"
#include "handler.h"
#include "../net.h"

namespace lily {
  namespace http {
    class Server {
     private:
      Ref<TCPServer> m_server;
      Handler m_handler;
      bool m_stop = false;
     public:
      Server() = default;
      static R<Ref<Server>, Error> Listen(const char *ip, uint16_t port, Handler handler) {
        auto server = New<Server>();
        server->m_handler = std::move(handler);
        Error err;
        tie(server->m_server, err) = TCPServer::ListenTCP(ip, port);
        if (err != NoError) {
          return {nullptr, err};
        }
        return {server, NoError};
      }
      void Stop() { m_stop = true; }
      void Serve();
    };
  }
}

#endif //LILY_SRC_NET_HTTP_SERVER_H_
