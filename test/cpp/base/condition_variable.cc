//
// Created by 28192 on 2024/8/2.
//
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool ready{false};



auto main() -> int {

  auto th1 = std::thread([](){
    std::unique_lock<std::mutex> lock(mtx);
    for (int i = 0; i < 7; ++i) {
      std::cout << "th1 sleep " << i << "\n";
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    cv.notify_one();
  });

  auto th2 = std::thread([](){
    std::unique_lock<std::mutex> lock(mtx);
    auto status = cv.wait_until(lock, std::chrono::system_clock::now() + std::chrono::seconds(2));
    if (status == std::cv_status::timeout) {
      std::cout << "++++++++++++++++++++++++timeout\n";
    } else {
      std::cout << "++++++++++++++++++++++++no timeout\n";
    }
  });


  th1.join();
  th2.join();

  return 0;
}