#ifndef LILY_SRC_COROUTINE_BIND_ONCE_H_
#define LILY_SRC_COROUTINE_BIND_ONCE_H_
/* BindOnce 产生一个只能调用一次的std::function<void()>对象，调用多次会产生非期望结果。
 * 由于std::bind需要对象可复制（具体来说，是绑定的对象可复制，其调用函数时，
 * 是以左值/左值引用的方式传递参数的[std::forward]，对于右值引用无法处理[因为函数要调用多次，
 * 通过右值引用传递，下一次调用会产生非期望的结果]），
 * 故对于一些只使用一次或者只有移动构造函数的对象，无法把它移动到std::bind对象内部，
 * 前者会产生额外的开销，后者则无法编译通过，故仿照std::bind，实现BindOnce，以便将右值引用传递给函数对象。
 */

template<typename Fn, typename ...Args>
struct _BindOnceResult {
  // 函数体
  Fn m_f;
  // 参数
  std::tuple<Args...> m_args;
  // 产生一个整数序列[0..sizeof...(Args)-1] from std::bind
  using it = typename std::_Build_index_tuple<sizeof...(Args)>::__type;
  // 全部使用std::move移动到内部。
  explicit _BindOnceResult(Fn &&fn, Args &&...args) :
      m_f(std::move(fn)),
      m_args(std::move(args)...) {}
  // 复制构造函数，通过const_cast与std::move强制使用移动构造函数，以将该结构体移动到std::function内部。
  _BindOnceResult(const _BindOnceResult &other) :
      m_f(std::move(const_cast<_BindOnceResult &>(other).m_f)),
      m_args(std::move(const_cast<_BindOnceResult &>(other).m_args)) {}
  // 解包m_args并调用函数。
  template<size_t ...indexes>
  void invoke(std::_Index_tuple<indexes...>) {
    m_f(std::move(std::get<indexes>(m_args))...);
  }
  void operator()() {
    invoke(it());
  }
};

// 产生一个只能调用一次的函数对象，传入的函数对象应当是可以被移动的。
template<typename Fn, typename ...Args>
_BindOnceResult<Fn, Args...> BindOnce(Fn &&fn, Args &&...args) {
  return _BindOnceResult<Fn, Args...>(std::forward<Fn>(fn), std::move(args)...);
}
#endif
