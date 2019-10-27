#ifndef LIBLILY_SRC_COROUTINE_DISPATCHER_H_
#define LIBLILY_SRC_COROUTINE_DISPATCHER_H_

/* Dispatcher / 分发器
 * 分发器可看作liblily的runtime，它负责分发Task到Manager中。
 * 使用单例模式保证全局只有一个分发器对象存在。
 * 在程序开始时，主线程将main函数作为第一个Task传入Manager，然后主线程进入epoll直到Task数为0时被唤醒。
 * 被唤醒后，等到该进程的线程数为0时，调用std::exit(0)正常退出。
 */

#include "singleton.h"
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
namespace lily {
  // 前向声明
  class Manager;
  class Epoller;
  // 分发器类
  class _Dispatcher {
   private:
    int m_thread_num;
    // Manager数组，用于结束时唤醒所有Manager。Manager在构造时将自己加入该数组。
    std::vector<Manager *> m_managers;
    // Task数
    std::atomic<int> m_task_count;
    // 当前序号，决定Task被分配到哪一个Manager中
    std::atomic<int> m_index;
    // 互斥锁，用于保护初期Manager的创建，使得新线程能正确地将对应的Manager加入到Manager数组中
    std::mutex m_managers_mtx;
    // 线程池，用于管理线程
    std::vector<std::thread> m_threads;
    // 线程池互斥锁
    std::mutex m_threads_mtx;
    // 用于等待所有线程结束。
    std::unique_ptr<Epoller> m_epoller;
    // 用于唤醒Epoll的eventfd
    int m_wake_up;
   public:
    _Dispatcher() noexcept;
    void TaskFinished();
    // 通过Epoll，直到所有Task完成前进入睡眠。
    void WaitAllFinished();
    // 判断是否所有任务均已结束
    bool AllTaskFinished() { return m_task_count == 0; }
    // 获取当前任务数
    __attribute__((unused))int GetTaskCount() { return m_task_count; }
    // 启动一个新Manager，并向该Manager传入第一个Task。
    void StartNewManager(std::function<void()> &&fn);
    // 分发Task
    void PutTask(std::function<void()> &&fn);
    // 启动分发器。
    void StartDispatcher(std::function<void()> &&main);
    int GetTaskCount() const { return m_task_count; }
  };
  using Dispatcher = Singleton<_Dispatcher>;
}

#endif
