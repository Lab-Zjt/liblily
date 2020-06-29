#ifndef LIBLILY_SRC_COROUTINE_TASK_H_
#define LIBLILY_SRC_COROUTINE_TASK_H_

#include <boost/context/execution_context_v2.hpp>
#include <functional>
#include <memory>
#include <sys/mman.h>

namespace lily {

  class StackTraits {
   public:
    // linux page size
    constexpr static size_t page_size_ = 4096;
    constexpr static size_t stack_size = 1024 * 1024;
    static bool is_unbounded() { return false; }

    static std::size_t page_size() { return sysconf(_SC_PAGESIZE); }

    static std::size_t default_size() { return stack_size; }

    static std::size_t minimum_size() { return stack_size; }

    static std::size_t maximum_size() { return stack_size; }
  };

  class StackAllocator {
    // 只会分配在虚拟内存中，如果没有用到的话不会加载到物理内存中，事实上没有太大的开销
    // 64位linux提供47位的用户空间虚拟内存地址，可分配2^27个1M的栈
    // 但如果只是短时间用了一次多余的空间也会实际占用到物理内存（以及虚拟页被换出后的磁盘空间）
   public:
    constexpr static size_t stack_size = StackTraits::stack_size;
    typedef StackTraits traits_type;
    boost::context::stack_context allocate() {
      void *vp = mmap(nullptr, stack_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
      boost::context::stack_context context;
      context.size = stack_size;
      context.sp = static_cast<char *>(vp) + context.size;
      return context;
    }
    void deallocate(boost::context::stack_context &context) {
      auto vp = static_cast<char *>(context.sp) - context.size;
      munmap(vp, stack_size);
    }
  };

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
    Task(Task &&other) noexcept:
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
