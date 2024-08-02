//
// Created by 28192 on 2024/8/1.
//
#define __cpp_lib_coroutine

#include <iostream>
#include <coroutine>
#include <future>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

struct Result {

  struct promise_type {
    std::suspend_never initial_suspend() {
      return {};
    }

    std::suspend_never final_suspend() noexcept {
      return {};
    }

    Result get_return_object() {
      return {};
    }

    void return_void() {

    }

//    void return_value(int value) {
//
//    }

    void unhandled_exception() {

    }
  };

};

struct Awaiter {
  int value;

  bool await_ready() {
    return false;
  }

  void await_suspend(std::coroutine_handle<> coroutine_handle) {
     auto as = std::async([=]() -> void {
      std::this_thread::sleep_for(1s);
      coroutine_handle.resume();
    });
  }

  int await_resume() {
    return value;
  }
};

auto Coroutine() -> Result {
  std::cout << 1 << '\n';
  std::cout << co_await Awaiter{.value = 1000} << '\n';
  std::cout << 2 << '\n';
  std::cout << 3 << '\n';
  co_await std::suspend_always{};
  std::cout << 4 << '\n';

  co_return;
};

auto main() -> int {
  Coroutine();
  return 0;
}