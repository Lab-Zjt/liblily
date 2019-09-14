#ifndef LIBLILY_SRC_COROUTINE_TASK_H_
#define LIBLILY_SRC_COROUTINE_TASK_H_

#include <boost/context/execution_context_v2.hpp>
#include <functional>
#include <memory>

namespace lily {
  // Task类
  class Task {
   private:
    using Context = boost::context::v2::execution_context<void>;
    using Function = std::function<void()>;
   public:
    // Task的状态
    enum Status { Ready, Running, Blocking, Finished };
   private:
    // Task要执行的函数
    Function m_callable;
    // 调用该协程的上下文
    Context m_from;
    // 协程上下文
    Context m_context;
    // 当前状态
    Status m_status = Ready;
   public:
    Task() = default;
    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;
    Task &operator=(Task &&) = delete;
    explicit Task(Function &&fn) noexcept;
    Task(Task &&other) noexcept :
        m_callable(std::move(other.m_callable)),
        m_from(std::move(other.m_from)),
        m_context(std::move(other.m_context)),
        m_status(other.m_status) {}
    // 重新执行该Task
    void Resume() { m_context = m_context(); }
    // 挂起当前Task
    void Yield() { m_from = m_from(); }
    Status GetStatus() { return m_status; }
    void SetStatus(Status s) { m_status = s; }
  };
}

#endif
