#ifndef LILY_SRC_APP_PROTO_PROTO_H_
#define LILY_SRC_APP_PROTO_PROTO_H_
#include <thread>
#include "../../common/common.h"
#include "../../net/net.h"
#include "../../yuri/proto/serialize.h"
#include "../../yuri/proto/deserialize.h"
#include "../../common/bufio.h"
#include "../../net/http/server.h"

using namespace reflect;
using namespace lily;

namespace proto {

  class NotifySender {
    Writer m_w;
   public:
    NotifySender(Writer w) : m_w(std::move(w)) {}
    template<typename Notify>
    Error SendNotify(const Notify &notify) {
      auto packet = proto::Serializer::Serialize(notify);
      auto[c, err] = m_w.Write(span(packet.data));
      ReturnIfError(err);
      return NoError;
    }
  };

  template<typename Notify>
  Error SendNotify(Writer w, const Notify &n) {
    return NotifySender(std::move(w)).SendNotify(n);
  }

  struct StartServerNotify {
   using_reflect(StartServerNotify)
   reflect_field(const int, cmd_id, = 1)
   reflect_field(std::string, addr, = "")
   reflect_field(uint16_t, port, = 0)
   reflect_field(std::string, root, = "")
  };

  struct LogNotify {
   using_reflect(LogNotify)
   reflect_field(const int, cmd_id, = 2)
   reflect_field(std::string, log, = "")
  };

  struct StopServerNotify {
   using_reflect(StopServerNotify)
   reflect_field(const int, cmd_id, = 3)
   reflect_field(std::string, reason, = "")
  };

  struct StartServerFailedNotify {
   using_reflect(StartServerFailedNotify)
   reflect_field(const int, cmd_id, = 4)
   reflect_field(std::string, reason, = "")
  };

  class LogWriter {
    Writer m_w;
   public:
    LogWriter(Writer w) : m_w(std::move(w)) {}
    R<ssize_t, Error> Write(span<char> log) {
      LogNotify notify;
      // trim \n
      notify.log = std::string(log.data(), log.size() - 1);
      return {log.size(), SendNotify(m_w, notify)};
    }
  };

  class ProtoHandler {
   public:
    using Handler = std::function<Error(const Packet &packet)>;
   private:
    std::unordered_map<int, Handler> m_handler;
   public:
    ProtoHandler() {}
    template<typename Notify>
    void RegisterHandler(std::function<Error(const Notify &)> &&fn) {
      Notify notify;
      m_handler[notify.cmd_id] = [fn = std::move(fn)](const Packet &packet) -> Error {
        Notify notify = Deserializer::Deserialize<Notify>(packet);
        return fn(notify);
      };
    }
    Error Handle(const ReadWriter &rw) {
      IntString str{};
      auto[c, err] = rw.Read(span(str.str, int_size));
      ReturnIfError(err);
      if (c < 4) {
        return Error{"read length failed"};
      }
      Packet packet;
      int msg_len = StrToInt(str.str);
      packet.data.resize(int_size + msg_len);
      packet.SetSize(msg_len);
      tie(c, err) = rw.Read(span(packet.data.data() + int_size, msg_len));
      ReturnIfError(err);
      auto cmd_id = Deserializer::GetFirstField<int>(packet);
      if (auto it = m_handler.find(cmd_id); it == m_handler.end()) {
        std::cout << "unknown proto id: " << cmd_id << "\n";
        return NoError;
      } else if (err = it->second(packet); err != NoError) {
        return err;
      }
      return NoError;
    }
  };
}

#endif //LILY_SRC_APP_PROTO_PROTO_H_
