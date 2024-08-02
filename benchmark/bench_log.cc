//
// Created by 28192 on 2024/7/31.
//
#include <benchmark/benchmark.h>
#include <pinecone/base/log.h>
#include <third_party/muduo/muduo/base/LogFile.h>
#include <third_party/muduo/muduo/base/Logging.h>
#include <third_party/muduo/muduo/base/Types.h>
#include <glog/logging.h>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/core.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <iostream>
#include <boost/core/null_deleter.hpp>
#include <fstream>
#include <boost/log/attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/exceptions.hpp>
#include <boost/log/keywords/format.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <benchmark/benchmark.h>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
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


static auto BmLogBoostLog(benchmark::State &state) -> void {
  int size = state.range(0);
  boost::log::core::get()->add_global_attribute("DateTime", boost::log::attributes::local_clock());
  boost::log::core::get()->add_global_attribute("ThreadId", boost::log::attributes::current_thread_id());
  boost::log::core::get()->add_global_attribute("Scope", boost::log::attributes::named_scope());
  auto sink = logging::add_file_log
      (
          keywords::file_name = "/tmp/boost_log_%N.log", // %N 是节点名
          keywords::rotation_size = 10 * 1024 * 1024, // 10MB
          keywords::format =
              (
                  expr::stream
                      << boost::log::trivial::severity << " "
                      << expr::if_(expr::has_attr<boost::posix_time::ptime>("DateTime"))
                      [
                          expr::stream << expr::format_date_time<boost::posix_time::ptime>("DateTime",
                                                                                           "%Y-%m-%d %H:%M:%S.%f")
                                       << " "
                      ]
                      << expr::if_(expr::has_attr<boost::log::attributes::current_thread_id::value_type>("ThreadId"))
                      [
                          expr::stream << expr::attr<boost::log::attributes::current_thread_id::value_type>("ThreadId")
                                       << " "
                      ]
                      << expr::format_named_scope("Scope", boost::log::keywords::format = "%F:%l")
                      << " " << expr::smessage
              )
      );
  BOOST_LOG_FUNCTION();

  for (auto _ : state) {
    for (int i = 0; i < size; ++i) {
      BOOST_LOG_TRIVIAL(trace) << "abcdefghijklmnopqrstuvwxyz._0123456789";
    }
  }
}

static auto BmLogMuduo(benchmark::State& state) -> void {
  int size = state.range(0);
  g_log_file.reset(new muduo::LogFile(::basename("/tmp/benchmark_muduo.log"), 10 * 1024 *1024));
  muduo::Logger::setOutput(OutputFunc);
  muduo::Logger::setFlush(FlushFunc);
  for (auto _ : state) {
//    BOOST_LOG_TRIVIAL(trace) << "abcdefghijklmnopqrstuvwxyz._0123456789";
    for (int i = 0; i < size; ++i) {
      LOG_INFO << "abcdefghijklmnopqrstuvwxyz._0123456789";
    }
  }
}

static auto BmLogGlog(benchmark::State &state) -> void {
  int size = state.range(0);

  for (auto _ : state) {
    for (int i = 0; i < size; ++i) {
      LOG(INFO) << "abcdefghijklmnopqrstuvwxyz._0123456789";
    }
  }
}

static auto BmLogPinecone(benchmark::State& state) -> void {
  using namespace pinecone::base;
  using namespace std;
  int size = state.range(0);

  auto logger = make_shared<Logger>();
  auto file_appender = make_shared<FileLogAppender>("/tmp/pinecone.log");
  file_appender->SetFormatter(
      make_shared<pinecone::base::LogFormatter>("%p %d %t %f:%l %m"));
  file_appender->SetLevel(pinecone::base::LogLevel::kTrace);

  logger->AddAppender(file_appender);
  for (auto _ : state) {
    for (int i = 0; i < size; ++i) {
      PINECONE_LOG_INFO(logger) << "abcdefghijklmnopqrstuvwxyz._0123456789";
    }
  }
}


BENCHMARK(BmLogBoostLog)->Arg(1)->Arg(10)->Arg(100);
BENCHMARK(BmLogMuduo)->Arg(1)->Arg(10)->Arg(100);
BENCHMARK(BmLogGlog)->Arg(1)->Arg(10)->Arg(100);
BENCHMARK(BmLogPinecone)->Arg(1)->Arg(10)->Arg(100);


auto main([[maybe_unused]] int argc,[[maybe_unused]] char** argv) -> int {
  google::InitGoogleLogging(argv[0]);

  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks();
  google::ShutdownGoogleLogging();
  return 0;
}