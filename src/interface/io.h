#ifndef IO_I
#define IO_I
#include "interface_basic.h"
#if __cplusplus < 201703ul
template <typename T, typename U>
std::shared_ptr<T> reinterpret_pointer_cast(const std::shared_ptr<U>& r) noexcept  {
  auto p = reinterpret_cast<typename std::shared_ptr<T>::element_type *>(r.get());
  return std::shared_ptr<T>(r, p);
}
#else
using std::reinterpret_pointer_cast;
#endif
class Reader{
  struct _Dummy{};
  template<typename _T>
  struct _Vtb {
    static _Vtb _vtb;
    R<ssize_t, Error>  (_T::* Read)(lily::span<char>  buf);
  };
  Ref<_Dummy> ptr;
  _Vtb<_Dummy>* vtb;
 public:
  template<typename _T>
  Reader(Ref<_T> t): ptr(reinterpret_pointer_cast<_Dummy>(std::move(t))), vtb(reinterpret_cast<_Vtb<_Dummy> *>(&_Vtb<_T>::_vtb)) {}
  R<ssize_t, Error>  Read(lily::span<char>  buf) {
    return ((ptr.get())->*(vtb->Read))(buf);
  }
};
template<typename _T>
Reader::_Vtb<_T> Reader::_Vtb<_T>::_vtb = {&_T::Read};
class Writer{
  struct _Dummy{};
  template<typename _T>
  struct _Vtb {
    static _Vtb _vtb;
    R<ssize_t, Error>  (_T::* Write)(lily::span<char>  buf);
  };
  Ref<_Dummy> ptr;
  _Vtb<_Dummy>* vtb;
 public:
  template<typename _T>
  Writer(Ref<_T> t): ptr(reinterpret_pointer_cast<_Dummy>(std::move(t))), vtb(reinterpret_cast<_Vtb<_Dummy> *>(&_Vtb<_T>::_vtb)) {}
  R<ssize_t, Error>  Write(lily::span<char>  buf) {
    return ((ptr.get())->*(vtb->Write))(buf);
  }
};
template<typename _T>
Writer::_Vtb<_T> Writer::_Vtb<_T>::_vtb = {&_T::Write};
class ReadWriter:public Reader, public Writer{
 public:
  template<typename _T>
  ReadWriter(Ref<_T> t): Reader(t),Writer(t) {}
};
class Closer{
  struct _Dummy{};
  template<typename _T>
  struct _Vtb {
    static _Vtb _vtb;
    Error  (_T::* Close)();
  };
  Ref<_Dummy> ptr;
  _Vtb<_Dummy>* vtb;
 public:
  template<typename _T>
  Closer(Ref<_T> t): ptr(reinterpret_pointer_cast<_Dummy>(std::move(t))), vtb(reinterpret_cast<_Vtb<_Dummy> *>(&_Vtb<_T>::_vtb)) {}
  Error  Close() {
    return ((ptr.get())->*(vtb->Close))();
  }
};
template<typename _T>
Closer::_Vtb<_T> Closer::_Vtb<_T>::_vtb = {&_T::Close};
class Seeker{
  struct _Dummy{};
  template<typename _T>
  struct _Vtb {
    static _Vtb _vtb;
    R<ssize_t, Error>  (_T::* Seek)(ssize_t  offset, int  whence);
  };
  Ref<_Dummy> ptr;
  _Vtb<_Dummy>* vtb;
 public:
  template<typename _T>
  Seeker(Ref<_T> t): ptr(reinterpret_pointer_cast<_Dummy>(std::move(t))), vtb(reinterpret_cast<_Vtb<_Dummy> *>(&_Vtb<_T>::_vtb)) {}
  R<ssize_t, Error>  Seek(ssize_t  offset, int  whence) {
    return ((ptr.get())->*(vtb->Seek))(offset, whence);
  }
};
template<typename _T>
Seeker::_Vtb<_T> Seeker::_Vtb<_T>::_vtb = {&_T::Seek};
class ReadSeeker:public Reader, public Seeker{
 public:
  template<typename _T>
  ReadSeeker(Ref<_T> t): Reader(t),Seeker(t) {}
};
class WriteSeeker:public Writer, public Seeker{
 public:
  template<typename _T>
  WriteSeeker(Ref<_T> t): Writer(t),Seeker(t) {}
};
class ReadWriteSeeker:public Reader, public Writer, public Seeker{
 public:
  template<typename _T>
  ReadWriteSeeker(Ref<_T> t): Reader(t),Writer(t),Seeker(t) {}
};
class ReadCloser:public Reader, public Closer{
 public:
  template<typename _T>
  ReadCloser(Ref<_T> t): Reader(t),Closer(t) {}
};
class WriteCloser:public Writer, public Closer{
 public:
  template<typename _T>
  WriteCloser(Ref<_T> t): Writer(t),Closer(t) {}
};
class ReadWriteCloser:public Reader, public Writer, public Closer{
 public:
  template<typename _T>
  ReadWriteCloser(Ref<_T> t): Reader(t),Writer(t),Closer(t) {}
};
class ReaderFrom{
  struct _Dummy{};
  template<typename _T>
  struct _Vtb {
    static _Vtb _vtb;
    R<ssize_t, Error>  (_T::* ReadFrom)(const Reader&  reader);
  };
  Ref<_Dummy> ptr;
  _Vtb<_Dummy>* vtb;
 public:
  template<typename _T>
  ReaderFrom(Ref<_T> t): ptr(reinterpret_pointer_cast<_Dummy>(std::move(t))), vtb(reinterpret_cast<_Vtb<_Dummy> *>(&_Vtb<_T>::_vtb)) {}
  R<ssize_t, Error>  ReadFrom(const Reader&  reader) {
    return ((ptr.get())->*(vtb->ReadFrom))(reader);
  }
};
template<typename _T>
ReaderFrom::_Vtb<_T> ReaderFrom::_Vtb<_T>::_vtb = {&_T::ReadFrom};
class WriterTo{
  struct _Dummy{};
  template<typename _T>
  struct _Vtb {
    static _Vtb _vtb;
    R<ssize_t, Error>  (_T::* WriteTo)(const Writer&  writer);
  };
  Ref<_Dummy> ptr;
  _Vtb<_Dummy>* vtb;
 public:
  template<typename _T>
  WriterTo(Ref<_T> t): ptr(reinterpret_pointer_cast<_Dummy>(std::move(t))), vtb(reinterpret_cast<_Vtb<_Dummy> *>(&_Vtb<_T>::_vtb)) {}
  R<ssize_t, Error>  WriteTo(const Writer&  writer) {
    return ((ptr.get())->*(vtb->WriteTo))(writer);
  }
};
template<typename _T>
WriterTo::_Vtb<_T> WriterTo::_Vtb<_T>::_vtb = {&_T::WriteTo};
#endif
