#ifndef LILY_SRC_COROUTINE_CO_MUTEX_H_
#define LILY_SRC_COROUTINE_CO_MUTEX_H_

#include <mutex>
#include <queue>
#include <atomic>
#include <sys/eventfd.h>
namespace lily {
  // 伪协程互斥锁，注意临界区内尽量不要有会导致阻塞的代码。
  class CoroutineMutex {
    std::mutex m_mtx;
   public:
    CoroutineMutex() = default;
    void lock();
    void unlock();
  };
  class CoroutineCond {
    int m_event = eventfd(0, 0);
   public:
    void wait(CoroutineMutex &mtx);
    void signal();
    int GetEventFd() const { return m_event; }
  };
}

#endif //LILY_SRC_COROUTINE_CO_MUTEX_H_
