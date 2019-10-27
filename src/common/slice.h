#ifndef LILY_SRC_NET_SLICE_H_
#define LILY_SRC_NET_SLICE_H_
#include <memory>

namespace lily {
  template<typename T>
  class span {
   public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using index_type = std::size_t;
    using diffrence_type = std::ptrdiff_t;
    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;
    using iterator = T *;
    using const_iterator = const T *;
   private:
    element_type *m_head;
    index_type m_size;
   public:
    span(T *head, std::size_t size) : m_head(head), m_size(size) {}
    template<typename Container>
    explicit span(Container &c) : m_head(c.data()), m_size(c.size()) {}
    template<typename Container>
    explicit span(const Container &c) : m_head(c.data()), m_size(c.size()) {}
    span(const span &s) = default;
    span &operator=(const span &) = default;
    iterator begin() { return m_head; }
    const_iterator begin() const { return m_head; }
    const_iterator cbegin() { return m_head; }
    iterator end() { return m_head + m_size; }
    const_iterator end() const { return m_head + m_size; }
    const_iterator cend() const { return m_head + m_size; }
    reference front() { return *m_head; }
    const_reference front() const { return *m_head; }
    reference back() { return *(m_head + m_size - 1); }
    const_reference back() const { return *(m_head + m_size - 1); }
    reference operator[](index_type i) { return m_head[i]; }
    const_reference operator[](index_type i) const { return m_head[i]; }
    pointer data() const { return m_head; }
    index_type size() const { return m_size; }
    index_type size_bytes() const { return m_size * sizeof(element_type); }
    bool empty() const { return m_size == 0; }
    span first(index_type i) const { return span(m_head, i); }
    span last(index_type i) const { return span(m_head + m_size - i, i); }
    span subspan(index_type off, index_type count) const { return span(m_head + off, count); }
    span sub(index_type from, index_type to) const { return span(m_head + from, to - from); }
    const char *as_chars() const { return reinterpret_cast<const char *>(m_head); }
    char *as_writable_chars() { return reinterpret_cast<char *>(m_head); }
  };
  template<typename Container>
  span(Container &) -> span<typename Container::value_type>;
  template<typename Container>
  span(const Container &) -> span<const typename Container::value_type>;

  template<typename T>
  class array {
   public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using index_type = std::size_t;
    using diffrence_type = std::ptrdiff_t;
    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;
    using iterator = T *;
    using const_iterator = const T *;
   private:
    std::unique_ptr<T[]> m_head;
    index_type m_size;
   public:
    explicit array(std::size_t size) : m_head(new T[size]), m_size(size) {}
    array(const array &s) = delete;
    array &operator=(const array &) = delete;
    iterator begin() { return m_head.get(); }
    const_iterator begin() const { return m_head.get(); }
    const_iterator cbegin() { return m_head.get(); }
    iterator end() { return m_head.get() + m_size; }
    const_iterator end() const { return m_head.get() + m_size; }
    const_iterator cend() const { return m_head.get() + m_size; }
    reference front() { return *m_head.get(); }
    const_reference front() const { return *m_head.get(); }
    reference back() { return *(m_head.get() + m_size - 1); }
    const_reference back() const { return *(m_head.get() + m_size - 1); }
    reference operator[](index_type i) { return m_head[i]; }
    const_reference operator[](index_type i) const { return m_head[i]; }
    pointer data() const { return m_head.get(); }
    index_type size() const { return m_size; }
    index_type size_bytes() const { return m_size * sizeof(element_type); }
    bool empty() const { return m_size == 0; }
    span<T> first(index_type i) const { return span(m_head.get(), i); }
    span<T> last(index_type i) const { return span(m_head.get() + m_size - i, i); }
    span<T> subspan(index_type off, index_type count) const { return span(m_head.get() + off, count); }
    span<T> sub(index_type from, index_type to) const { return span(m_head.get() + from, to - from); }
    const char *as_chars() const { return reinterpret_cast<const char *>(m_head.get()); }
    char *as_writable_chars() { return reinterpret_cast<char *>(m_head.get()); }
    span<T> as_span() { return span(m_head.get(), m_size); }
  };

}

//namespace lily {
//  template<typename T, typename E = typename std::enable_if<std::is_copy_constructible<T>::value>::type>
//  class Slice {
//   private:
//    static size_t pow2(size_t n) {
//      return 1ul << n;
//    }
//
//    static size_t log2(unsigned long long n) {
//      int is_pow2 = __builtin_popcountll(n) == 1 ? 1 : 0;
//      return (sizeof(unsigned long long) - is_pow2) - __builtin_clzll(n);
//    }
//
//    static size_t get_next_size(size_t size) {
//      return pow2(log2(size) + 1);
//    }
//   private:
//    std::shared_ptr<T[]> m_raw = nullptr;
//    T *m_data = nullptr;
//    size_t m_len = 0;
//    T *m_storage_end = nullptr;
//   private:
//    Slice extend() {
//      Slice res(get_next_size(cap()));
//      res.copy_from(m_data, m_len);
//      return res;
//    }
//    void copy_from(const T *data, size_t len) {
//      std::copy(data, data + len, m_data);
//      m_len = len;
//    }
//    void push(const T &t) { m_data[m_len++] = t; }
//    void push(T &&t) { m_data[m_len++] = std::move(t); }
//   public:
//    Slice() = default;
//    explicit Slice(size_t size) :
//        m_raw(new T[size]),
//        m_data(m_raw.get()),
//        m_len(size),
//        m_storage_end(m_raw.get() + size) {}
//    Slice append(const T &t) {
//      if (m_len >= cap()) {
//        Slice res = this->extend();
//        res.push(t);
//        return res;
//      } else {
//        this->push(t);
//        return *this;
//      }
//    }
//    Slice append(T &&t) {
//      if (len() >= cap()) {
//        Slice res = this->extend();
//        res.push(std::move(t));
//        return res;
//      } else {
//        this->push(std::move(t));
//        return *this;
//      }
//    }
//    T *data() { return m_data; }
//    const T *data() const { return m_data; }
//    size_t len() const { return m_len; }
//    size_t cap() const { return m_storage_end - m_data; }
//    T *begin() { return m_data; }
//    const T *begin() const { return m_data; }
//    T *end() { return m_data + len(); }
//    const T *end() const { return m_data + len(); }
//    T &operator[](int i) { return data()[i]; }
//    const T &operator[](int i) const { return data()[i]; }
//    // 创建一个共享底层数组的Slice
//    Slice sub(size_t from, size_t to) {
//      Slice res;
//      res.m_raw = m_raw;
//      res.m_data = m_data + from;
//      res.m_len = to - from;
//      res.m_storage_end = m_storage_end;
//      return res;
//    }
//    Slice ref(size_t from, size_t to) {
//      Slice res;
//      res.m_data = m_data + from;
//      res.m_len = to - from;
//      res.m_storage_end = m_storage_end;
//      return res;
//    }
//  };
//
//  using Bytes = Slice<char>;
//
//  std::string string(const Bytes &bytes) {
//    return std::string(bytes.data(), bytes.len());
//  }
//}

#endif //LILY_SRC_NET_SLICE_H_
