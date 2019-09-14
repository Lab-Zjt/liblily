#ifndef LILY_SRC_COROUTINE_CO_TIMER_H_
#define LILY_SRC_COROUTINE_CO_TIMER_H_
#include <chrono>
#include "co_chan.h"
#include "go.h"
namespace lily {
  using Duration = int64_t;
  constexpr Duration NanoSecond = 1;
  constexpr Duration MicroSecond = 1000 * NanoSecond;
  constexpr Duration MilliSecond = 1000 * MicroSecond;
  constexpr Duration Second = 1000 * MilliSecond;
  constexpr Duration Minute = 60 * Second;
  constexpr Duration Hour = 60 * Minute;
  constexpr Duration Day = 24 * Hour;
  constexpr Duration Week = 7 * Day;

  enum Month {
    January, February, March, April, May, June, July, August, September, October, November, December
  };

  enum Weekday {
    Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday
  };

  struct Timer : std::enable_shared_from_this<Timer> {
    Channel<Duration> C;
    bool Stop = false;
    explicit Timer(Duration after) {
      auto ptr = shared_from_this();
      go([after, ptr]() {
        Duration d = std::chrono::high_resolution_clock::now().time_since_epoch().count() + after;
        usleep(after / MicroSecond);
        if (ptr->Stop)return;
        ptr->C << d;
      });
    }
  };

  struct Ticker : std::enable_shared_from_this<Ticker> {
    Channel<Duration> C;
    bool Stop = false;
    Ticker(Duration after) {
      auto ptr = shared_from_this();
      go([after, ptr]() {
        auto next = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        for (;;) {
          Duration d = next + after;
          usleep(after / MicroSecond);
          if (ptr->Stop)break;
          ptr->C << d;
        }
      });
    }
  };
}

#endif //LILY_SRC_COROUTINE_CO_TIMER_H_