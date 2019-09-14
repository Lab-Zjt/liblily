#ifndef LIBLILY_SRC_COROUTINE_MANAGER_H_
#define LIBLILY_SRC_COROUTINE_MANAGER_H_

#include "epoller.h"
#include "singleton.h"
#include <queue>
#include <mutex>
#include <memory>
#include <functional>

namespace lily {

  struct ManagerInit {
    bool init = false;
    int mgr_epoll_fd = -1;
  };
  class Task;

  // Manager类
  class Manager {
   private:
    // Task的前体，由于构造Task时需要当前线程的Manager的ucontext，故保存构造Task需要的参数。
    using PreparedTask = std::function<void()>;
   private:
    // Task队列，轮流执行Task。
    std::queue<std::unique_ptr<Task>> m_tasks;
    // Manager对应的Task。
    std::unique_ptr<Task> m_this_task;
    // 当前执行的Task。
    Task *m_cur_task;
    // 存放PreparedTask的数组。
    std::vector<PreparedTask> m_prepared_tasks;
    // 存放Epoll事件的数组。
    std::vector<epoll_event> m_epoll_event;
    // Epoller
    Epoller m_epoller;
    // 当前线程的id。
    pthread_t m_id;
    // 将PreparedTask从数组中取出来时防止竞争的mutex。
    std::mutex m_tasks_mtx;
    // 用于唤醒当前线程的eventfd。
    int m_wake_up;
    // 标识是否进入无期限epoll_wait。
    bool m_nothing_to_do;
   public:
    Manager() noexcept;
    void Init() {
      ThreadLocal<ManagerInit>::Get().init = true;
      ThreadLocal<ManagerInit>::Get().mgr_epoll_fd = m_epoller.GetFd();
    }
    __attribute__((unused)) bool IsInitFinished() { return ThreadLocal<ManagerInit>::Get().init; }
    __attribute__((unused)) pthread_t ID() { return m_id; }
    __attribute__((unused)) Task *ThisTask() { return m_this_task.get(); }
    Task *CurrentTask() { return m_cur_task; }
    // 将一个文件描述符加入epoll监听队列中。
    void AddListenFd(int fd, epoll_event *ev);
    // 将一个文件描述符从epoll监听队列中移除。
    void DelListenFd(int fd);
    // 将PreparedTask加入到数组中。
    void PutTask(PreparedTask &&p);
    // 从PreparedTask数组中取出PreparedTask并构造为Task。
    void LoadTasks();
    // 唤醒该Manager。
    void WakeUp();
    // 将wake up文件描述符加入监听队列中，进入睡眠状态，直到有事件到来或该Manager被唤醒。
    int Sleep();
    // 被唤醒时，移除wake up文件描述符。
    void OnAwake();
    // 等待任务到来
    void WaitForTask();
    // 更新Task状态。
    void RenewTasks();
    // 入口点
    void Start();
  };

}

#endif
