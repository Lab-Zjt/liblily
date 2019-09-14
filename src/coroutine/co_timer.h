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

  struct Timer {
    Channel<Duration> C;
    bool Stop = false;
    static std::shared_ptr<Timer> New(Duration after);
  };

  struct Ticker {
    Channel<Duration> C;
    bool Stop = false;
    static std::shared_ptr<Ticker> New(Duration after);
  };
}

#endif //LILY_SRC_COROUTINE_CO_TIMER_H_