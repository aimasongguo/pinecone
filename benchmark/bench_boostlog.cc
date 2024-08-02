//
// Created by 28192 on 2024/7/31.
//
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

static auto BmLog(benchmark::State &state) -> void {
  int size = state.range(0);
  boost::log::core::get()->add_global_attribute("DateTime", boost::log::attributes::local_clock());
  boost::log::core::get()->add_global_attribute("ThreadId", boost::log::attributes::current_thread_id());
  boost::log::core::get()->add_global_attribute("Scope", boost::log::attributes::named_scope());
  auto sink = logging::add_file_log
      (
          keywords::file_name = "logs/app_%N.log", // %N 是节点名
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

BENCHMARK(BmLog)->Arg(1)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();