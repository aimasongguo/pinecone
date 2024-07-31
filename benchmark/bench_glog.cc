//
// Created by 28192 on 2024/7/31.
//
#include <glog/logging.h>
#include <benchmark/benchmark.h>

static auto BmLog(benchmark::State& state) -> void {
  for (auto _ : state) {
    LOG(INFO) << "abcdefghijklmnopqrstuvwxyz._0123456789";
  }
}

BENCHMARK(BmLog);

auto main([[maybe_unused]] int argc,[[maybe_unused]] char** argv) -> int {
  google::InitGoogleLogging(argv[0]);
  // 设置日志输出到stderr，而不是默认的文件
//  FLAGS_logtostderr = true;

  // 设置日志级别为INFO（默认是INFO，但这里明确设置）
//  FLAGS_stderrthreshold = 0;  // 0 = INFO, 1 = WARNING, 2 = ERROR

  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks();

  google::ShutdownGoogleLogging();
  return 0;
}