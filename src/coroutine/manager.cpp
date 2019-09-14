#include "manager.h"
#include "task.h"
#include "dispatcher.h"
#include <sys/eventfd.h>
namespace lily {
  Manager::Manager() noexcept :
      m_tasks(),
      m_this_task(new Task([this]() { Start(); })),
      m_cur_task(),
      m_prepared_tasks(),
      m_epoll_event(16),
      m_epoller(),
      m_id(pthread_self()),
      m_tasks_mtx(),
      m_wake_up(eventfd(0, 0)),
      m_nothing_to_do(false) {}
  void Manager::AddListenFd(int fd, epoll_event *ev) {
    m_epoller.Add(fd, ev);
  }
  void Manager::DelListenFd(int fd) {
    m_epoller.Del(fd);
  }
  void Manager::PutTask(lily::Manager::PreparedTask &&p) {
    std::lock_guard<std::mutex> lock(m_tasks_mtx);
    m_prepared_tasks.push_back(std::move(p));
  }
  void Manager::LoadTasks() {
    std::vector<PreparedTask> temp;
    {
      std::lock_guard<std::mutex> lock(m_tasks_mtx);
      temp.swap(m_prepared_tasks);
    }

    // 如果有新任务到来，重置m_nothing_to_to为false。
    if (!temp.empty())m_nothing_to_do = false;
    for (auto &&p: temp) {
      m_tasks.push(std::unique_ptr<Task>(new Task(std::move(p))));
    }
  }
  void Manager::WakeUp() {
    uint64_t wp = 1;
    write(m_wake_up, &wp, sizeof(wp));
  }
  int Manager::Sleep() {
    epoll_event ev[1];
    ev->data.ptr = this;
    ev->events = EPOLLIN;
    m_epoller.Add(m_wake_up, ev);
    return m_epoller.Epoll(m_epoll_event);
  }
  void Manager::OnAwake() {
    uint64_t wp = 0;
    m_epoller.Del(m_wake_up);
    read(m_wake_up, &wp, sizeof(wp));
  }
  void Manager::WaitForTask() {
    auto ready = Sleep();
    // 此次唤醒是Dispatcher唤醒的，所有任务均已结束，故结束该线程。
    if (ready == 1 && Dispatcher::Get().AllTaskFinished()) {
      pthread_exit(nullptr);
    }
    for (int i = 0; i < ready; ++i) {
      if (m_epoll_event[i].data.ptr == this)OnAwake();
      else static_cast<Task *>(m_epoll_event[i].data.ptr)->SetStatus(Task::Ready);
    }
  }
  // 更新Task状态，对于有事件到来的Task，将其状态更新为Ready。
  void Manager::RenewTasks() {
    auto ready = m_epoller.Epoll(m_epoll_event, 0);
    // 如果事件较多，一次处理不完，则进行扩容。
    if (ready == m_epoll_event.size()) {
      m_epoll_event.resize(m_epoll_event.size() * 2);
    }
    for (int i = 0; i < ready; ++i) {
      if (m_epoll_event[i].data.ptr == this)OnAwake();
      else static_cast<Task *>(m_epoll_event[i].data.ptr)->SetStatus(Task::Ready);
    }
  }
  void Manager::Start() {
    try {
      while (true) {
        // 从PreparedTask数组中加载Task
        LoadTasks();
        // 如果当前Task队列为空或全部进入Blocking状态，则进入无期限epoll_wait状态，否则进行无阻塞epoll_wait，只更新Task状态。
        if (m_tasks.empty() || m_nothing_to_do) {
          WaitForTask();
        } else {
          RenewTasks();
        }
        auto cur_size = m_tasks.size();
        m_nothing_to_do = true;
        while (cur_size-- > 0) {
          // 取出一个Task
          auto cur = std::move(m_tasks.front());
          m_tasks.pop();
          // Ready状态： 执行该Task，然后放回队列尾
          // Blocking状态： 直接放回队列尾
          // Finished状态： 不做处理
          // Running状态： 出现异常
          if (cur->GetStatus() == Task::Ready) {
            m_nothing_to_do = false;
            cur->SetStatus(Task::Running);
            m_cur_task = cur.get();
            cur->Resume();
            m_cur_task = nullptr;
            m_tasks.push(std::move(cur));
          } else if (cur->GetStatus() == Task::Blocking) {
            m_tasks.push(std::move(cur));
          } else if (cur->GetStatus() == Task::Finished) {}
          else { fprintf(stderr, "FATAL ERROR: TASK YIELD WITH RUNNING STATUS\n"); }
        }
      }
    }
    catch (std::exception &e) {
      printf("%s Throw Exception: %s\n", __PRETTY_FUNCTION__, e.what());
    }
  }
}