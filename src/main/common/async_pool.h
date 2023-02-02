#ifndef A6_SRC_MAIN_COMMON_ASYNC_POOL_H_
#define A6_SRC_MAIN_COMMON_ASYNC_POOL_H_

#include <functional>
#include <future>
#include <thread>

#include "timer.h"

namespace s21 {

struct Info {
  std::future<void> future;
  Timestamp delete_time;
  bool remove = false;
};

class AsyncPool {
 public:
  AsyncPool() { Manager(); };
  ~AsyncPool() { stop_ = true; }
  AsyncPool(const AsyncPool&) = delete;
  AsyncPool(AsyncPool&&) = delete;
  void operator==(const AsyncPool&) = delete;
  void operator==(AsyncPool&&) = delete;

  template <typename Task, typename... Args>
  size_t DelayTask(seconds const& sec, Task const& task, Args... args) {
    auto task_func = std::bind(task, args...);

    std::scoped_lock<std::mutex> lock(mtx_);
    while (tasks_.find(last_id_) != tasks_.end()) ++last_id_;
    auto& info = tasks_[last_id_] = {{}, Clock::now() + sec};

    info.future = std::async(std::launch::async, [&, task_func] {
      while (true) {
        std::this_thread::sleep_for(0.1s);
        std::scoped_lock<std::mutex> lock(mtx_);
        if (info.remove || stop_ || info.delete_time <= Clock::now()) break;
      }

      if (!info.remove && !stop_) task_func();
    });

    return last_id_++;
  }

  seconds GetRemainTime(size_t id) const {
    std::scoped_lock<std::mutex> lock(mtx_);
    auto itr = tasks_.find(id);

    if (itr != tasks_.end()) {
      auto sec = duration_cast<seconds>(itr->second.delete_time - Clock::now());

      if (sec > 0s) return sec;
    }

    return 0s;
  }

  void StopTask(size_t id) {
    std::scoped_lock<std::mutex> lock(mtx_);
    if (tasks_.find(id) != tasks_.end()) tasks_[id].remove = true;
  }

 private:
  std::unordered_map<size_t, Info> tasks_;
  std::future<void> garbage_collector_;
  std::atomic_size_t last_id_ = 0;
  std::atomic_bool stop_ = false;
  mutable std::mutex mtx_;

  void Manager() {
    auto func = [&] {
      for (auto i = tasks_.begin(); i != tasks_.end(); ++i) {
        if (i->second.future.wait_for(0s) == std::future_status::ready) {
          i = tasks_.erase(i);
          if (i == tasks_.end()) break;
        }
      }
    };

    garbage_collector_ = std::async(std::launch::async, [&, func] {
      while (!stop_) {
        std::this_thread::sleep_for(0.3s);
        std::scoped_lock<std::mutex> lock(mtx_);

        func();
      }
    });
  }
};

}  // namespace s21

#endif  // A6_SRC_MAIN_COMMON_ASYNC_POOL_H_
