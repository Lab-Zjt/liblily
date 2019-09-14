#include "co_timer.h"

namespace lily {
  std::shared_ptr<Timer> Timer::New(lily::Duration after) {
    auto ptr = std::make_shared<Timer>();
    go([after, ptr]() {
      Duration d = std::chrono::high_resolution_clock::now().time_since_epoch().count() + after;
      usleep(after / MicroSecond);
      if (ptr->Stop)return;
      ptr->C << d;
    });
    return ptr;
  }

  std::shared_ptr<Ticker> Ticker::New(lily::Duration after) {
    auto ptr = std::make_shared<Ticker>();
    go([after, ptr]() {
      for (;;) {
        Duration d = std::chrono::high_resolution_clock::now().time_since_epoch().count() + after;
        usleep(after / MicroSecond);
        if (ptr->Stop)break;
        ptr->C << d;
      }
    });
    return ptr;
  }
}