#ifndef LIBLILY_SRC_COROUTINE_EPOLLER_H_
#define LIBLILY_SRC_COROUTINE_EPOLLER_H_

#include <vector>
#include <sys/epoll.h>

namespace lily {
  // 简单的Epoll Wrapper
  class Epoller {
   private:
    int m_fd;
   private:
    int Control(int fd, int ctl, epoll_event *ev);
   public:
    Epoller() noexcept ;
    ~Epoller();
    int Add(int fd, epoll_event *ev) ;
    int Del(int fd) ;
    __attribute__((unused)) int Mod(int fd, epoll_event *ev) ;
    int Epoll(std::vector<epoll_event> &event_list, int timeout = -1);
    int GetFd()const { return m_fd;}
  };
}

#endif
