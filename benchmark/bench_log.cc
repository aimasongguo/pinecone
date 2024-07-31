//
// Created by 28192 on 2024/7/31.
//
#include <benchmark/benchmark.h>
#include <pinecone/base/log.h>

static auto BmLog(benchmark::State& state) -> void {
  using namespace pinecone::base;
  using namespace std;
  auto logger = make_shared<Logger>();
  auto file_appender = make_shared<FileLogAppender>("./test.log");
  file_appender->SetFormatter(
      make_shared<pinecone::base::LogFormatter>("%p %d %t %f:%l %m"));
  file_appender->SetLevel(pinecone::base::LogLevel::kTrace);

  logger->AddAppender(file_appender);
  for (auto _ : state) {
    PINECONE_LOG_INFO(logger) << "abcdefghijklmnopqrstuvwxyz._0123456789";
  }
}

BENCHMARK(BmLog);

auto main([[maybe_unused]] int argc,[[maybe_unused]] char** argv) -> int {

  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks();

  return 0;
}