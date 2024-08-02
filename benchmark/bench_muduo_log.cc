//
// Created by 28192 on 2024/7/31.
//

#include <third_party/muduo/muduo/base/LogFile.h>
#include <third_party/muduo/muduo/base/Logging.h>
#include "third_party/muduo/muduo/base/Types.h"
#include <benchmark/benchmark.h>
#include <unistd.h>

#include <iostream>
#include <cstring>
#include <string>

std::unique_ptr<muduo::LogFile> g_log_file;

void OutputFunc(const char* msg, int len)
{
  g_log_file->append(msg, len);
}

void FlushFunc()
{
  g_log_file->flush();
}

static auto BmLog(benchmark::State& state) -> void {
  int size = state.range(0);
  g_log_file.reset(new muduo::LogFile(::basename("./benchmark_muduo.log"), 10 * 1024 *1024));
  muduo::Logger::setOutput(OutputFunc);
  muduo::Logger::setFlush(FlushFunc);
  for (auto _ : state) {
//    BOOST_LOG_TRIVIAL(trace) << "abcdefghijklmnopqrstuvwxyz._0123456789";
    for (int i = 0; i < size; ++i) {
      LOG_INFO << "abcdefghijklmnopqrstuvwxyz._0123456789";
    }
  }
}
BENCHMARK(BmLog)->Arg(1)->Arg(10)->Arg(100);

BENCHMARK_MAIN();