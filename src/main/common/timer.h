#ifndef A6_SRC_MAIN_COMMON_TIMER_H_
#define A6_SRC_MAIN_COMMON_TIMER_H_

#include <chrono>

using namespace std::chrono;
using namespace std::chrono_literals;

using Clock = high_resolution_clock;
using Timestamp = Clock::time_point;

namespace s21 {

class Timer {
 public:
  void Start() { timestamp_ = GetTime(); }
  nanoseconds Finish() { return GetTime() - timestamp_; }

 private:
  Timestamp timestamp_ = GetTime();

  Timestamp GetTime() { return high_resolution_clock::now(); }
};

}  // namespace s21

#endif  // A6_SRC_MAIN_COMMON_TIMER_H_
