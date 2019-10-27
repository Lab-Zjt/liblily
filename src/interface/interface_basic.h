#ifndef _INTERFACE_BASIC_
#define _INTERFACE_BASIC_
#include "../common/slice.h"
#include <memory>
#include <tuple>
#include <cstring>
#define ERRNO Error{strerror(errno)}
template<typename T, typename ...Args>
inline std::shared_ptr<T> New(Args &&...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}
template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T>
using Ptr = std::unique_ptr<T>;
template<typename T>
using WeakRef = std::weak_ptr<T>;
template<typename ...Ts>
using R = std::tuple<Ts...>;
struct Error {
  const char *desc = nullptr;
};
inline bool operator==(const Error &lhs, const Error &rhs) { return lhs.desc == rhs.desc; }
inline bool operator!=(const Error &lhs, const Error &rhs) { return lhs.desc != rhs.desc; }
static const Error NoError;
static const Error EndOfFile{"EOF"};
#endif