#include "hook.h"
#include "../coroutine/dispatcher.h"
#include "../coroutine/manager.h"
#include "../coroutine/task.h"
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
using namespace lily;
extern "C"
{
int usleep(useconds_t usec) {
  if (usec == 0) return 0;
  HOOK_CHECK(usleep);
  auto mgr = &ThreadLocal<Manager>::Get();
  if (mgr == nullptr) {
    return original_usleep(usec);
  }
  auto cur = mgr->CurrentTask();
  if (cur == nullptr) {
    return original_usleep(usec);
  }
  int fd = timerfd_create(CLOCK_REALTIME, 0);
  itimerspec spec{};
  spec.it_value.tv_sec = usec / 1000000;
  spec.it_value.tv_nsec = usec * 1000 % 1000000000;
  epoll_event ev{};
  ev.data.ptr = cur;
  ev.events = EPOLLIN;
  timerfd_settime(fd, 0, &spec, nullptr);
  mgr->AddListenFd(fd, &ev);
  cur->SetStatus(Task::Blocking);
  mgr->CurrentTask()->Yield();
  mgr->DelListenFd(fd);
  close(fd);
  return 0;
}

unsigned int sleep(unsigned int seconds) {
  if (seconds == 0) return 0;
  HOOK_CHECK(sleep);
  auto mgr = &ThreadLocal<Manager>::Get();
  if (mgr == nullptr) {
    return original_sleep(seconds);
  }
  auto cur = mgr->CurrentTask();
  if (cur == nullptr) {
    return original_sleep(seconds);
  }
  int fd = timerfd_create(CLOCK_REALTIME, 0);
  itimerspec spec{};
  spec.it_value.tv_sec = seconds;
  epoll_event ev{};
  ev.data.ptr = cur;
  ev.events = EPOLLIN;
  timerfd_settime(fd, 0, &spec, nullptr);
  mgr->AddListenFd(fd, &ev);
  cur->SetStatus(Task::Blocking);
  mgr->CurrentTask()->Yield();
  mgr->DelListenFd(fd);
  close(fd);
  return 0;
}
int nanosleep(const struct timespec *req, struct timespec *rem) {
  if (req->tv_nsec == 0 && req->tv_sec == 0)return 0;
  HOOK_CHECK(nanosleep);
  auto mgr = &ThreadLocal<Manager>::Get();
  if (mgr == nullptr) {
    return original_nanosleep(req, rem);
  }
  auto cur = mgr->CurrentTask();
  if (cur == nullptr) {
    return original_nanosleep(req, rem);
  }
  int fd = timerfd_create(CLOCK_REALTIME, 0);
  itimerspec spec{};
  spec.it_value = *req;
  epoll_event ev{};
  ev.data.ptr = cur;
  ev.events = EPOLLIN;
  timerfd_settime(fd, 0, &spec, nullptr);
  mgr->AddListenFd(fd, &ev);
  cur->SetStatus(Task::Blocking);
  mgr->CurrentTask()->Yield();
  *rem = spec.it_value;
  mgr->DelListenFd(fd);
  close(fd);
  return 0;
}
}