//
// Created by 28192 on 2024/8/1.
//
#include <fmt/core.h>
#include <fmt/chrono.h> // 如果你使用的是fmt 7.0或更高版本，它提供了对std::chrono的直接支持
#include <chrono>
#include <iostream>

int main() {
  // 获取当前时间点
  auto now = std::chrono::system_clock::now();

  // 转换为time_t（如果你使用的是fmt 7.0之前的版本，你可能需要这样做）
  // std::time_t now_c = std::chrono::system_clock::to_time_t(now);

  // 使用fmt库格式化时间（fmt 7.0及以后版本）
  std::string formatted_time = fmt::format("当前时间: {}", fmt::format_time("{}", now, "%Y-%m-%d %H:%M:%S"));

  // 如果你使用的是fmt 7.0之前的版本，并且转换为了time_t
  // std::string formatted_time = fmt::format("当前时间: {}", std::ctime(&now_c));

  std::cout << formatted_time << std::endl;

  return 0;
}