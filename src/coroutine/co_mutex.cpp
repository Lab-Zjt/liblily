#include "co_mutex.h"
#include "go.h"
namespace lily {
  void CoroutineMutex::lock() {
    while (!m_mtx.try_lock()) { yield(); }
  }
  void CoroutineMutex::unlock() { m_mtx.unlock(); }
  void CoroutineCond::wait(CoroutineMutex &mtx) {
    size_t buf;
    mtx.unlock();
    read(m_event, &buf, sizeof(buf));
    mtx.lock();
  }
  void CoroutineCond::signal() {
    size_t buf = 1;
    write(m_event, &buf, sizeof(buf));
  }
}