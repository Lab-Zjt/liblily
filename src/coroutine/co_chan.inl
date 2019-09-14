#include "co_chan.h"

namespace lily {
  template<typename T>
  void Channel<T>::operator<<(const T &data) {
    m_mtx.lock();
    while (m_queue.size() >= m_capacity) { m_cv.wait(m_mtx); }
    m_queue.push(data);
    // 由于push前为0,可能有正在等待读的线程，故唤醒
    if (m_queue.size() == 1) {
      m_cv.signal();
    }
    m_mtx.unlock();
  }
  template<typename T>
  T Channel<T>::operator()() {
    m_mtx.lock();
    while (m_queue.empty()) { m_cv.wait(m_mtx); }
    T tmp = std::move(m_queue.front());
    m_queue.pop();
    // 由于pop之前达到容量上限，可能有等待写的线程，故唤醒
    if (m_queue.size() == m_capacity - 1) {
      m_cv.signal();
    }
    m_mtx.unlock();
    return tmp;
  }
  template<typename T>
  void Channel<T>::operator<<(T &&data) {
    m_mtx.lock();
    while (m_queue.size() >= m_capacity) { m_cv.wait(m_mtx); }
    m_queue.emplace(std::move(data));
    if (m_queue.size() == 1) {
      m_cv.signal();
    }
    m_mtx.unlock();
  }
  template<typename T>
  void Channel<T>::operator>>(T &rhs) {
    m_mtx.lock();
    while (m_queue.empty()) { m_cv.wait(m_mtx); }
    rhs = std::move(m_queue.front());
    m_queue.pop();
    if (m_queue.size() == m_capacity - 1) {
      m_cv.signal();
    }
    m_mtx.unlock();
  }
  template<typename T>
  std::unique_ptr<T> Channel<T>::try_fetch() {
    std::lock_guard<CoroutineMutex> lock(m_mtx);
    // 被唤醒，但元素可能已被取走
    if (m_queue.empty()) { return nullptr; }
    std::unique_ptr<T> result(new T(std::move(m_queue.front())));
    m_queue.pop();
    if (m_queue.size() == m_capacity - 1) {
      m_cv.signal();
    }
    return result;
  }
}