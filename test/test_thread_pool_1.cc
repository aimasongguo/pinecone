//
// Created by 28192 on 2024/8/2.
//
#include <atomic>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <functional>

class ThreadPool {
 public:
  ThreadPool(const ThreadPool &) = delete;

  ThreadPool &operator=(const ThreadPool &) = delete;

  static ThreadPool &Instance() {
    static ThreadPool ins;
    return ins;
  }

  using Task = std::packaged_task<void()>;

  ~ThreadPool() {
    Stop();
  }

  template<class F, class... Args>
  auto Commit(F &&f, Args &&... args) -> std::future<decltype(f(args...))> {
    using RetType = decltype(f(args...));
    if (stop_.load()) {
      return std::future<RetType>{};
    }
    auto task = std::make_shared<std::packaged_task<RetType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<RetType> ret = task->get_future();
    {
      std::lock_guard<std::mutex> cv_mt(cv_mt_);
      tasks_.emplace([task] { (*task)(); });
    }
    cv_lock_.notify_one();
    return ret;
  }

  auto IdleThreadCount() -> uint32_t {
    return thread_num_;
  }

 private:
  explicit ThreadPool()
      : stop_(false), thread_num_(std::thread::hardware_concurrency()) {
    Start();
  }

  auto Start() -> void {
    for (uint32_t i = 0; i < thread_num_; ++i) {
      pool_.emplace_back([this]() {
        while (!this->stop_.load()) {
          Task task;
          {
            std::unique_lock<std::mutex> cv_mt(cv_mt_);
            this->cv_lock_.wait(cv_mt, [this] {
              return this->stop_.load() || !this->tasks_.empty();
            });
            if (this->tasks_.empty()) {
              return;
            }
            task = std::move(this->tasks_.front());
            this->tasks_.pop();
          }
          this->thread_num_--;
          task();
          this->thread_num_++;
        }
      });
    }
  }

  auto Stop() -> void {
    stop_.store(true);
    cv_lock_.notify_all();
    for (auto &thread : pool_) {
      if (thread.joinable()) {
        std::cout << "join thread " << thread.get_id() << '\n';
        thread.join();
      }
    }
  }


  std::mutex cv_mt_;
  std::condition_variable cv_lock_;
  std::atomic_bool stop_;
  std::atomic_uint32_t thread_num_;
  std::queue<Task> tasks_;
  std::vector<std::thread> pool_;
};


auto main() -> int {
  auto f = ThreadPool::Instance().Commit([]() {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 33;
  });

  if (f.wait_for(std::chrono::seconds(2)) != std::future_status::timeout) {
    std::cout << "no timeout\n";
  } else {
    std::cout << "timeout\n";
  }

  return 0;
}