#ifndef LIBLILY_SRC_COROUTINE_SINGLETON_H_
#define LIBLILY_SRC_COROUTINE_SINGLETON_H_
#include <cstdint>
namespace lily {
  template<typename T>
  class Singleton {
   public:
    Singleton(const Singleton &) = delete;
    static T &Get() {
      static T ins{};
      return ins;
    }
  };
  template<typename T>
  class ThreadLocal {
   public:
    ThreadLocal(const ThreadLocal &) = delete;
    static T &Get() {
      thread_local static T ins{};
      return ins;
    }
  };
}

#endif //LIBLILY_SRC_COROUTINE_SINGLETON_H_
