#ifndef LIBLILY_SRC_HOOK_HOOK_H_
#define LIBLILY_SRC_HOOK_HOOK_H_

#include <sys/types.h>
#include <sys/socket.h>
#define HOOK_DECLARE(func, ret, ...) typedef ret (*func##_t)(__VA_ARGS__);\
                                    __attribute__((unused)) thread_local static func##_t original_##func = nullptr
#define HOOK_CHECK(name)\
if (original_##name == nullptr) \
  original_##name = (decltype(original_##name))(dlsym(libc, #name));\
  if (original_##name == nullptr)printf("Hook failed: %s\n", #name),exit(-1)

struct sockaddr;
extern "C" {
extern void *libc;
void hookLibc();
HOOK_DECLARE(epoll_wait, int, int epfd, struct epoll_event *events,
             int maxevents, int timeout);
// Input Library Functions
HOOK_DECLARE(read, ssize_t, int fd, void *buf, size_t count);
HOOK_DECLARE(__read_alias, ssize_t, int fd, void *buf, size_t count);
HOOK_DECLARE(__read_chk, ssize_t, int fd, void *buf, size_t count, size_t buflen);
HOOK_DECLARE(__read_chk_warn, ssize_t, int fd, void *buf, size_t count, size_t buflen);
HOOK_DECLARE(recv, ssize_t, int sockfd, void *buf, size_t len, int flags);
HOOK_DECLARE(__recv_alias, ssize_t, int sockfd, void *buf, size_t len, int flags);
HOOK_DECLARE(recvfrom, ssize_t, int sockfd, void *buf, size_t len,
             int flags, struct sockaddr *src_addr, socklen_t *addrlen);
HOOK_DECLARE(recvmsg, ssize_t, int sockfd, struct msghdr *msg, int flags);
HOOK_DECLARE(recvmmsg, int, int sockfd, struct mmsghdr *msgvec, unsigned int vlen,
             int flags, struct timespec *timeout);
// Output Library Functions
HOOK_DECLARE(write, ssize_t, int fd, const void *buf, size_t count);
HOOK_DECLARE(send, ssize_t, int sockfd, const void *buf, size_t len, int flags);
HOOK_DECLARE(sendto, ssize_t, int sockfd, const void *buf, size_t len,
             int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
HOOK_DECLARE(sendmsg, ssize_t, int sockfd, const struct msghdr *msg, int flags);
HOOK_DECLARE(sendmmsg, int, int sockfd, struct mmsghdr *msgvec, unsigned int vlen, int flags);
// Connect / Accept
HOOK_DECLARE(connect, int, int sockfd, const struct sockaddr *addr, socklen_t addrlen);
HOOK_DECLARE(accept, int, int sockfd, struct sockaddr *addr, socklen_t *addrlen);
HOOK_DECLARE(accept4, int, int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
// Socket ( set socket O_NONBLOCK)
HOOK_DECLARE(socket, int, int domain, int type, int protocol);
HOOK_DECLARE(close, int, int fd);
// Sleep
HOOK_DECLARE(usleep, int, useconds_t usec);
HOOK_DECLARE(sleep, unsigned int, unsigned int seconds);
HOOK_DECLARE(nanosleep, int, const struct timespec *req, struct timespec *rem);
}

#endif //LIBLILY_SRC_HOOK_HOOK_H_
