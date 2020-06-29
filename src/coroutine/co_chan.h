#ifndef LILY_SRC_COROUTINE_CO_CHAN_H_
#define LILY_SRC_COROUTINE_CO_CHAN_H_

#include "co_mutex.h"
#include <queue>
#include <memory>
namespace lily {
  template<typename T>
  class Channel {
   public:
    using value_type = T;
   private:
    std::queue<T> m_queue;
    CoroutineCond m_cv;
    CoroutineMutex m_mtx;
    size_t m_capacity = std::numeric_limits<size_t>::max();
   public:
    Channel() = default;
    explicit Channel(size_t capacity) : m_capacity(capacity) {}
    Channel(const Channel &) = delete;
    Channel &operator=(const Channel &) = delete;
    // 判断当前Channel是否为空
    bool Empty() { return m_queue.empty(); }
    // 将data复制并放入Channel中
    inline void operator<<(const T &data);
    // 从Channel中取出一个数据
    inline T operator()();
    // 将data移动到Channel中
    inline void operator<<(T &&data);
    // 从Channel中取出一个数据并赋值给rhs
    inline void operator>>(T &rhs);
    int GetEventFd() const { return m_cv.GetEventFd(); }
    std::unique_ptr<T> try_fetch();
  };
}
#include "co_chan.inl"

#endif //LILY_SRC_COROUTINE_CO_CHAN_H_
