#ifndef LIBYURI_SRC_PROTO_H_
#define LIBYURI_SRC_PROTO_H_
#include "reflect.h"
#include "proto.h"
#include <type_traits>
namespace proto {

  class Serializer {
   private:
    using SerializeFunc = Any(*)(const void *);
    inline static std::unordered_map<reflect::TypeID, SerializeFunc> serialize_map;
    template<typename T>
    static Any SerializeImpl(const void *p) {
      Any any{};
      if constexpr (std::is_same_v<T, std::string>) {
        auto &t = *static_cast<const std::string *>(p);
        any.data.reserve(int_size * 2 + t.size());
        any.data += IntToStr(Type::String);
        any.data += IntToStr(t.size());
        any.data += t;
      } else if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>) {
        any.data.reserve(int_size * 2 + sizeof(T));
        any.data += IntToStr(Type::Number);
        any.data += IntToStr(sizeof(T));
        any.data.append(static_cast<const char *>(p), sizeof(T));
      } else if constexpr (std::is_pointer_v<T>) {
        auto &t = *static_cast<const T *>(p);
        using element_type = std::remove_pointer_t<T>;
        if (t == nullptr) {
          any.data += IntToStr(Type::Null);
          any.data += IntToStr(0);
        } else {
          any.data += SerializeImpl<element_type>(t).data;
        }
      } else if constexpr(reflect::is_shared_ptr_v<T>) {
        auto &t = *static_cast<const T *>(p);
        using element_type = typename T::element_type;
        if (t == nullptr) {
          any.data += IntToStr(Type::Null);
          any.data += IntToStr(0);
        } else {
          any.data += SerializeImpl<element_type>(t.get());
        }
      } else if constexpr (reflect::is_iterable_v<T>) {
        auto &t = *static_cast<const T *>(p);
        using V = typename T::value_type;
        any.data += IntToStr(Type::Array);
        any.data += IntToStr(t.size());
        for (auto &&elem : t) {
          any.data += SerializeImpl<V>(&elem).data;
        }
      } else if constexpr (reflect::is_pair_v<T>) {
        auto &t = *static_cast<const T *>(p);
        using K1 = typename T::first_type;
        using K2 = typename T::second_type;
        any.data += IntToStr(Type::Object);
        any.data += IntToStr(2);
        any.data += SerializeImpl<K1>(&t.first).data;
        any.data += SerializeImpl<K2>(&t.second).data;
      } else if constexpr (reflect::is_reflect_v<T>) {
        auto &t = *static_cast<const T *>(p);
        const std::vector<reflect::FieldInfo> &info_vec = t.get_field_info_vec();
        any.data += IntToStr(Type::Object);
        any.data += IntToStr(info_vec.size());
        for (auto &&info : info_vec) {
          any.data += serialize_map[info.type_id](static_cast<const char * >(p) + info.offset).data;
        }
      } else if constexpr(!reflect::is_reflect_v<T>) {
        static_assert(reflect::is_reflect_v<T>, "can not serialize");
      }
      return any;
    }
   public:
    template<typename T>
    static void RegisterHandler() {
      serialize_map[reflect::type_id<T>] = SerializeImpl<T>;
    }
    template<typename T>
    static Packet Serialize(const T &t) {
      Packet packet;
      packet.data += SerializeImpl<T>(&t).data;
      packet.SetSize(packet.data.size() - 4);
      return packet;
    }
  };

}

#endif
