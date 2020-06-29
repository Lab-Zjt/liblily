#include "hook.h"
#include "../coroutine/dispatcher.h"
#include "../coroutine/task.h"
#include "../coroutine/manager.h"
#include <dlfcn.h>
#include <sys/epoll.h>
using namespace lily;
enum IOType { Input = EPOLLIN, Output = EPOLLOUT };
template<IOType io, typename SignFn>
struct IOHook;
template<IOType io, typename Ret, typename ...Args>
struct IOHook<io, Ret(*)(int, Args...)> {
  Ret operator()(Ret(*original)(int, Args...), int fd, Args... args) {
    if (!ThreadLocal<ManagerInit>::Get().init) { return original(fd, args...); }
    auto mgr = &ThreadLocal<Manager>::Get();
    auto cur = mgr->CurrentTask();
    if (cur == nullptr) { return original(fd, args...); }
    Ret res{};
    errno = 0;
    res = original(fd, args...);
    if (res != -1 || errno != EAGAIN) {
      return res;
    }
    epoll_event ev{};
    ev.events = io;
    ev.data.ptr = cur;
    mgr->AddListenFd(fd, &ev);
    cur->SetStatus(Task::Blocking);
    cur->Yield();
    errno = 0;
    res = original(fd, args...);
    auto save_errno = errno;
    mgr->DelListenFd(fd);
    errno = save_errno;
    return res;
  }
};
extern "C" {
__attribute__((unused))const char *libc_name = "libc.so.6";
__attribute__((unused))void *libc = nullptr;
void hookLibc() {
  if (libc == nullptr) {
    libc = dlopen(libc_name, RTLD_LAZY);
    if (libc == nullptr) {
      fprintf(stderr, "ERROR: open %s failed.\n", libc_name);
      std::exit(-1);
    }
  }
}
ssize_t read(int fd, void *buf, size_t count) {
  HOOK_CHECK(read);
  return IOHook<Input, read_t>()(original_read, fd, buf, count);
}
/*ssize_t __read_alias(int fd, void *buf, size_t count) {
  HOOK_CHECK(read);
  printf("__read_alias called.\n");
  return IOHook<Input, read_t>()(original_read, fd, buf, count);
}*/
/*ssize_t __read_chk(int fd, void *buf, size_t nbytes,
                   size_t buflen) {
  HOOK_CHECK(__read_chk);
  printf("__read_chk called.\n");
  return IOHook<Input, __read_chk_t>()(original___read_chk, fd, buf, nbytes, buflen);
}*/
/*ssize_t __read_chk_warn(int fd, void *buf, size_t nbytes,
                        size_t buflen) {
  HOOK_CHECK(__read_chk_warn);
  printf("__read_chk_warn called.\n");
  return IOHook<Input, __read_chk_warn_t>()(original___read_chk_warn, fd, buf, nbytes, buflen);
}*/
ssize_t write(int fd, const void *buf, size_t count) {
  HOOK_CHECK(write);
  return IOHook<Output, write_t>()(original_write, fd, buf, count);
}
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  HOOK_CHECK(connect);
  if (!ThreadLocal<ManagerInit>::Get().init) {
    return original_connect(sockfd, addr, addrlen);
  }
  auto mgr = &ThreadLocal<Manager>::Get();
  auto cur = mgr->CurrentTask();
  if (cur == nullptr) {
    return original_connect(sockfd, addr, addrlen);
  }
  errno = 0;
  auto res = original_connect(sockfd, addr, addrlen);
  if (errno == EINPROGRESS) {
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.ptr = cur;
    mgr->AddListenFd(sockfd, &ev);
    cur->SetStatus(Task::Blocking);
    mgr->CurrentTask()->Yield();
    errno = 0;
    res = original_connect(sockfd, addr, addrlen);
    auto save_errno = errno;
    if (errno == EISCONN) {
      res = 0;
    }
    mgr->DelListenFd(sockfd);
    errno = save_errno;
  }
  cur->SetStatus(Task::Running);
  return res;
}
int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) {
  HOOK_CHECK(accept4);
  if (!ThreadLocal<ManagerInit>::Get().init) {
    return original_accept4(sockfd, addr, addrlen, flags);
  }
  auto mgr = &ThreadLocal<Manager>::Get();
  auto cur = mgr->CurrentTask();
  if (cur == nullptr) {
    return original_accept4(sockfd, addr, addrlen, flags);
  }
  epoll_event ev{};
  ev.events = EPOLLIN;
  ev.data.ptr = cur;
  mgr->AddListenFd(sockfd, &ev);
  cur->SetStatus(Task::Blocking);
  mgr->CurrentTask()->Yield();
  errno = 0;
  int fd = original_accept4(sockfd, addr, addrlen, (unsigned) flags | SOCK_NONBLOCK);
  auto save_errno = errno;
  mgr->DelListenFd(sockfd);
  errno = save_errno;
  return fd;
}
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
  return accept4(sockfd, addr, addrlen, SOCK_NONBLOCK);
}
ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
  HOOK_CHECK(recv);
  return IOHook<Input, recv_t>()(original_recv, sockfd, buf, len, flags);
}
ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
  HOOK_CHECK(send);
  return IOHook<Output, send_t>()(original_send, sockfd, buf, len, flags);
}
ssize_t recvfrom(int sockfd, void *buf, size_t len,
                 int flags, struct sockaddr *dest_addr, socklen_t *addrlen) {
  HOOK_CHECK(recvfrom);
  auto ret = IOHook<Input, recvfrom_t>()(original_recvfrom, sockfd, buf, len, flags, dest_addr, addrlen);
  return ret;
}
ssize_t sendto(int sockfd, const void *buf, size_t len,
               int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
  HOOK_CHECK(sendto);
  return IOHook<Output, sendto_t>()(original_sendto, sockfd, buf, len, flags, dest_addr, addrlen);
}
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
  HOOK_CHECK(recvmsg);
  return IOHook<Input, recvmsg_t>()(original_recvmsg, sockfd, msg, flags);
}
ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
  HOOK_CHECK(sendmsg);
  return IOHook<Output, sendmsg_t>()(original_sendmsg, sockfd, msg, flags);
}
int recvmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen, int flags, struct timespec *timeout) {
  HOOK_CHECK(recvmmsg);
  return IOHook<Input, recvmmsg_t>()(original_recvmmsg, sockfd, msgvec, vlen, flags, timeout);
}
int sendmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen, int flags) {
  HOOK_CHECK(sendmmsg);
  return IOHook<Output, sendmmsg_t>()(original_sendmmsg, sockfd, msgvec, vlen, flags);
}
int socket(int domain, int type, int protocol) noexcept {
  HOOK_CHECK(socket);
  if (!ThreadLocal<ManagerInit>::Get().init) {
    return original_socket(domain, type, protocol);
  }
  auto mgr = &ThreadLocal<Manager>::Get();
  auto cur = mgr->CurrentTask();
  if (cur == nullptr) {
    return original_socket(domain, type, protocol);
  }
  int sock = original_socket(domain, (unsigned) type | SOCK_NONBLOCK, protocol);
  return sock;
}
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) {
  HOOK_CHECK(epoll_wait);
  if (!ThreadLocal<ManagerInit>::Get().init || ThreadLocal<ManagerInit>::Get().mgr_epoll_fd == epfd) {
    return original_epoll_wait(epfd, events, maxevents, timeout);
  }
  auto mgr = &ThreadLocal<Manager>::Get();
  auto cur = mgr->CurrentTask();
  if (cur == nullptr) {
    return original_epoll_wait(epfd, events, maxevents, timeout);
  }
  auto ready = original_epoll_wait(epfd, events, maxevents, 0);
  if (ready > 0 || timeout == 0) {
    return ready;
  }
  epoll_event ev[1];
  ev->events = EPOLLIN;
  ev->data.ptr = cur;
  mgr->AddListenFd(epfd, ev);
  cur->SetStatus(Task::Blocking);
  cur->Yield();
  errno = 0;
  ready = original_epoll_wait(epfd, events, maxevents, timeout);
  auto save_errno = errno;
  mgr->DelListenFd(epfd);
  errno = save_errno;
  return ready;
}
}
