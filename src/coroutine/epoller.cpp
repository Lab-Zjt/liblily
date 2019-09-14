#include "epoller.h"
#include <unistd.h>
#include <cstdio>
namespace lily {
  int Epoller::Control(int fd, int ctl, epoll_event *ev) {
    return epoll_ctl(m_fd, ctl, fd, ev);
  }
  Epoller::Epoller() noexcept {
    m_fd = epoll_create(1);
    if (m_fd < 0) perror("epoll_create");
  }
  Epoller::~Epoller() {
    close(m_fd);
  }
  int Epoller::Add(int fd, epoll_event *ev) {
    return Control(fd, EPOLL_CTL_ADD, ev);
  }
  int Epoller::Del(int fd) {
    return Control(fd, EPOLL_CTL_DEL, nullptr);
  }
  __attribute__((unused)) int Epoller::Mod(int fd, epoll_event *ev) {
    return Control(fd, EPOLL_CTL_MOD, ev);
  }
  int Epoller::Epoll(std::vector<epoll_event> &event_list, int timeout) {
    return epoll_wait(m_fd, event_list.data(), event_list.size(), timeout);
  }
}