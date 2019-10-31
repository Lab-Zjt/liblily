#ifndef LIBLILY_SRC_COROUTINE_GO_H_
#define LIBLILY_SRC_COROUTINE_GO_H_

#include "dispatcher.h"
#include "manager.h"
#include "task.h"
#include "bind_once.h"

// 通过go，将函数转换成PreparedTask，置入Manager的PreparedTask队列中。
template<typename Fn, typename ...Args>
inline void go(Fn &&fn, Args &&...args) {
  lily::Dispatcher::Get().PutTask(
      std::function<void()>{BindOnce(std::forward<Fn>(fn), std::move(args)...)}
  );
}
inline __attribute__((unused)) void yield() {
  auto manager = &lily::ThreadLocal<lily::Manager>::Get();
  manager->CurrentTask()->SetStatus(lily::Task::Ready);
  manager->CurrentTask()->Yield();
}

inline __attribute__((unused)) void exit_all() {
  lily::Dispatcher::Get().Cancel();
}
#include "entry.h"
#endif //LIBLILY_SRC_COROUTINE_GO_H_
