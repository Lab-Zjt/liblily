#include "task.h"
#include "manager.h"
#include "dispatcher.h"
namespace lily {
  Task::Task(lily::Task::Function &&fn) noexcept:
      m_callable(std::move(fn)),
      m_from(),
      m_context() {
    if (!ThreadLocal<ManagerInit>::Get().init) {
      m_context = Context(std::allocator_arg_t{}, StackAllocator{}, [this](Context &&sink) {
        m_callable();
        return sink();
      });
    } else {
      auto mgr = &ThreadLocal<Manager>::Get();
      m_context = Context(std::allocator_arg_t{}, StackAllocator{}, [mgr](Context &&sink) {
        mgr->CurrentTask()->m_from = std::move(sink);
        mgr->CurrentTask()->m_callable();
        mgr->CurrentTask()->m_status = Finished;
        Dispatcher::Get().TaskFinished();
        return mgr->CurrentTask()->m_from();
      });
    }
  }
}