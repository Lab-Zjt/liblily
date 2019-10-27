#include "dispatcher.h"
#include "setting.h"
#include "manager.h"
#include "task.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <signal.h>

namespace lily {
  _Dispatcher::_Dispatcher() noexcept :
      m_thread_num(MaxThreadNum),
      m_managers(),
      m_task_count(0),
      m_index(0),
      m_managers_mtx(),
      m_threads(),
      m_threads_mtx(),
      m_epoller(new Epoller),
      m_wake_up(eventfd(0, 0)) {
    // 预留空间，避免重分配带来的迭代器失效。
    m_managers.reserve(m_thread_num);
  }
  void _Dispatcher::TaskFinished() {
    // Task结束时调用该函数，使得Task数减1,Task数为0时唤醒主线程。
    auto c = --m_task_count;
    if (c == 0) {
      uint64_t wp = 1;
      write(m_wake_up, &wp, sizeof(wp));
    }
  }
  void _Dispatcher::WaitAllFinished() {
    std::vector<epoll_event> ev(1);
    ev[0].data.ptr = this;
    ev[0].events = EPOLLIN;
    m_epoller->Add(m_wake_up, &ev[0]);
    while (m_epoller->Epoll(ev) == -1) {}
    m_epoller->Del(m_wake_up);
  }
  void _Dispatcher::StartNewManager(std::function<void()> &&fn) {
    std::lock_guard<std::mutex> lock(m_threads_mtx);
    m_threads.emplace_back(std::thread([this](std::function<void()> &&fn) {
      m_managers_mtx.lock();
      m_managers.emplace_back(&ThreadLocal<Manager>::Get());
      m_managers_mtx.unlock();
      ThreadLocal<Manager>::Get().Init();
      ThreadLocal<Manager>::Get().PutTask(std::move(fn));
      ThreadLocal<Manager>::Get().Start();
    }, std::move(fn)));
  }
  void _Dispatcher::PutTask(std::function<void()> &&fn) {
    // 令Task数和序号加1。
    auto c = m_task_count++;
    auto this_index = m_index++;
    // 如果Manager数还未达到上限，新开启一个Manager。
    // 否则，直到对应的Manager启动为止，该线程进入睡眠。
    // 然后将任务放入Manager中，并尝试唤醒Manager。
    if (this_index < m_thread_num) {
      StartNewManager(std::move(fn));
    } else {
      int manager_index = this_index % m_thread_num;
      while (manager_index >= m_managers.size())pthread_yield();
      while (m_managers[manager_index] == nullptr)pthread_yield();
      m_managers[manager_index]->PutTask(std::move(fn));
      m_managers[manager_index]->WakeUp();
    }
  }
  void _Dispatcher::StartDispatcher(std::function<void()> &&main) {
    signal(SIGUSR1, [](int sig) {});
    signal(SIGPIPE, SIG_IGN);
    // 将main作为第一个Task传入。
    PutTask(std::move(main));
    // 等待所有Task结束。
    WaitAllFinished();
    if (m_task_count == 0) {
      // 唤醒所有Manager，以便Manager发现所有Task结束并退出其线程。
      for (const auto &m :m_managers) {
        m->WakeUp();
      }
      // 等待所有线程退出
      for (auto &&th:m_threads) {
        th.join();
      }
      std::exit(0);
    } else {
      printf("Fatal Error: Task Count Become 1 Again\n");
      std::exit(-1);
    }
  }
}