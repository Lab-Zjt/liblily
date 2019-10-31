#ifndef LIBYURI_SRC_PROTO_DESERIALIZE_H_
#define LIBYURI_SRC_PROTO_DESERIALIZE_H_

#include <cstring>
#include "proto.h"

namespace proto {

  class Deserializer {
   private:

    static Type GetType(const std::string &str, int &off) {
      int res = StrToInt(str.data() + off);
      off += int_size;
      return static_cast<Type>(res);
    }

    static int GetSize(const std::string &str, int &off) {
      int res = StrToInt(str.data() + off);
      off += int_size;
      return res;
    }

    using DeserializeFunc = bool (*)(const std::string &, int &, void *, bool, Type, int);

    inline static std::unordered_map<reflect::TypeID, DeserializeFunc> deserialize_map;

    template<typename T>
    static bool DeserializeImpl(const std::string &str,
                                int &off,
                                void *ptr,
                                bool parse_type = true,
                                Type type = static_cast<Type >(0),
                                int s = 0) {
      if (parse_type) {
        type = GetType(str, off);
        s = GetSize(str, off);
      }
      if constexpr (std::is_same_v<std::string, T>) {
        if (type != String) {
          fprintf(stderr, "error: parse string type failed.\n");
          return false;
        }
        std::string &res = *static_cast<std::string *>(ptr);
        res = str.substr(off, s);
        off += s;
        return true;
      } else if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>) {
        if (type != Number) {
          fprintf(stderr, "error: parse number type failed.\n");
          return false;
        }
        if (s != sizeof(T)) {
          fprintf(stderr, "error: parse number int_size failed.\n");
          return false;
        }
        for (int i = 0; i < sizeof(T); ++i) {
          static_cast<char *>(ptr)[i] = str[off + i];
        }
        off += sizeof(T);
        return true;
      } else if constexpr (reflect::is_shared_ptr_v<T>) {
        using V = typename T::value_type;
        std::shared_ptr<V> &res = *static_cast<std::shared_ptr<V> *>(ptr);
        if (type == Null) {
          res = nullptr;
          return true;
        }
        return DeserializeImpl<V>(str, off, res.get(), false, type, s);
      } else if constexpr (reflect::is_iterable_v<T>) {
        using V = typename T::value_type;
        T &res = *static_cast<T *>(ptr);
        V v;
        for (int i = 0; i < s; ++i) {
          if (!DeserializeImpl<V>(str, off, &v)) {
            fprintf(stderr, "parse iterable element failed.\n");
            return false;
          }
          res.insert(res.end(), std::move(v));
        }
        return true;
      } else if constexpr (reflect::is_pair_v<T>) {
        using K1 = typename std::remove_cv_t<typename T::first_type>;
        using K2 = typename T::second_type;
        T &res = *static_cast<T *>(ptr);
        if (!DeserializeImpl<K1>(str, off, reinterpret_cast<void *>(const_cast<K1 *>(&res.first)))) {
          fprintf(stderr, "parse pair first failed.\n");
          return false;
        }
        if (!DeserializeImpl<K2>(str, off, reinterpret_cast<void *>(&res.second))) {
          fprintf(stderr, "parse pair second failed.\n");
          return false;
        }
        return true;
      } else if constexpr (reflect::is_reflect_v<T>) {
        const std::vector<reflect::FieldInfo> &info_vec = T::get_field_info_vec();
        if (s != info_vec.size()) {
          fprintf(stderr, "parse reflect field int_size failed. field count is %lu, but get %d.\n", info_vec.size(), s);
          return false;
        }
        for (auto &&info : info_vec) {
          if (!deserialize_map[info.type_id](str,
                                             off,
                                             static_cast<char *>(ptr) + info.offset,
                                             true,
                                             static_cast<Type >(0),
                                             0)) {
            fprintf(stderr, "parse reflect field failed.\n");
            return false;
          };
        }
        return true;
      } else {
        static_assert(!reflect::is_reflect_v<T>, "can not deserialize");
      }
    }
    static bool IgnoreField(const std::string &str, int &off) {
      auto type = GetType(str, off);
      auto s = GetSize(str, off);
      switch (type) {
        case Type::String:
        case Type::Number:
        case Type::Null: {
          off += s;
          break;
        }
        case Type::Array :
        case Type::Object: {
          while (s-- > 0) {
            IgnoreField(str, off);
          }
        }
      }
      return true;
    }
   public:

    template<typename T>
    static void RegisterHandler() {
      deserialize_map[reflect::type_id<T>] = DeserializeImpl<T>;
    }

    template<typename T>
    static T Deserialize(const Packet &packet) {
      int off = int_size;
      T t;
      if (!DeserializeImpl<T>(packet.data, off, &t)) {
        fprintf(stderr, "parse packet failed.\n");
        return {};
      };
      return t;
    }
    template<typename T>
    static T GetFirstField(const Packet &packet) {
      int off = int_size;
      auto type = GetType(packet.data, off);
      if (type != Type::Object) {
        fprintf(stderr, "get first field failed: not a object.\n");
        return {};
      }
      auto s = GetSize(packet.data, off);
      if (s < 1) {
        fprintf(stderr, "get first field failed: require at least 1 field.\n");
        return {};
      }
      T t;
      if (!DeserializeImpl<T>(packet.data, off, &t)) {
        fprintf(stderr, "get first field failed: deserialize failed.\n");
        return {};
      }
      return t;
    }
  };
}

#endif
