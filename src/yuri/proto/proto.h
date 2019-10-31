#ifndef LIBYURI_SRC_PROTO_PROTO_H_
#define LIBYURI_SRC_PROTO_PROTO_H_

namespace proto {
  // int type;
  // int length;
  // char data[];
  struct Any {
    std::string data;
  };

  enum Type {
    String = 1,
    Number = 2,
    Array = 3,
    Object = 4,
    Null = 5
  };

  constexpr inline const size_t int_size = sizeof(int);

  struct IntString {
    char str[int_size];
  };

  constexpr IntString IntToStr(int i) {
    union {
      int num;
      IntString str;
    } u{i};
    return u.str;
  }

  int StrToInt(const char *str) {
    union {
      IntString str;
      int i;
    } u{};
    for (int i = 0; i < int_size; ++i) {
      u.str.str[i] = str[i];
    }
    return u.i;
  }

  std::string &operator+=(std::string &str, IntString arr) {
    str.append(arr.str, int_size);
    return str;
  }

  struct Packet : Any {
    Packet() { data += IntToStr(0); }
    void SetSize(int size) {
      auto str = IntToStr(size);
      for (int i = 0; i < int_size; ++i) { data[i] = str.str[i]; }
    }
  };
}

#endif //LIBYURI_SRC_PROTO_PROTO_H_
