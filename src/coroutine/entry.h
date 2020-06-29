#ifndef LIBLILY_SRC_COROUTINE_ENTRY_H_
#define LIBLILY_SRC_COROUTINE_ENTRY_H_
// 程序入口点
#include "../hook/hook.h"
#define Main(a, b) int _cmain(int,char**);\
int main(int argc,char* argv[]){\
  hookLibc();\
  lily::Dispatcher::Get().StartDispatcher([argc,argv](){_cmain(argc,argv);});\
  return 0;\
}\
int _cmain(__attribute__((unused)) a,__attribute__((unused)) b)
#endif
