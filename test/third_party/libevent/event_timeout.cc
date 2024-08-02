//
// Created by 28192 on 2024/8/2.
//

#include <event2/event.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

std::mutex mutex;
std::condition_variable cv;

// 回调函数，当定时器事件触发时调用
void TimeoutCb(evutil_socket_t  /*fd*/, short  /*event*/, void *arg) {
  std::cout << "任务超时，没有完成" << '\n';

  // 停止事件循环（这里假设我们只有一个事件循环）
  event_base_loopbreak(static_cast<struct event_base *>(arg));

  // 注意：这里并没有真正的异步任务在执行，只是用定时器来模拟
  // 在实际应用中，你可能需要在这里处理异步任务的结果或超时逻辑
}

auto main() -> int {
  struct event_base *base = event_base_new();
  if (base == nullptr) {
    std::cerr << "无法创建事件循环" << '\n';
    return 1;
  }

  std::thread([&](){
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_until(std::move(lock), );
  }).detach();

  // 创建一个一次性定时器事件
  struct event *event = event_new(base, -1, EV_TIMEOUT | EV_PERSIST, TimeoutCb, static_cast<void *>(base));
  if (event == nullptr) {
    std::cerr << "无法创建定时器事件" << '\n';
    event_base_free(base);
    return 1;
  }

  // 设置定时器超时时间为2秒
  struct timeval two_sec = {2, 0};
  event_add(event, &two_sec);

  // 进入事件循环
  event_base_dispatch(base);

  // 清理资源
  event_free(event);
  event_base_free(base);

  return 0;
}