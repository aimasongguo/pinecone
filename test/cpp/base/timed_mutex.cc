//
// Created by 28192 on 2024/8/2.
//
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>

//void Func() {
//  // 尝试在1秒内获得锁
//  if (mtx.try_lock_for(std::chrono::seconds(1))) {
//    std::cout << "Lock acquired successfully\n";
//
//    // 模拟一些工作
//    std::this_thread::sleep_for(std::chrono::seconds(2));
//
//    mtx.unlock(); // 显式解锁（虽然在这里使用std::lock_guard或std::unique_lock会更安全）
//  } else {
//    std::cout << "Failed to acquire lock within 1 second\n";
//  }
//}

auto main() -> int {
  {
    std::timed_mutex mtx;
    auto th1 = std::jthread([&](){
      std::unique_lock<std::timed_mutex> lock(mtx);
      for (int i = 0; i < 5; ++i) {
        std::cout << "th1 sleep " << i << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1L));
      }
    });

    auto th2 = std::jthread([&](){
      if (!mtx.try_lock_for(std::chrono::seconds(2L))) {
        std::cout << "++++++++++++++++++++++++timeout\n";
      } else {
        std::cout << "++++++++++++++++++++++++no timeout\n";
      }
    });
  }
  return 0;
}