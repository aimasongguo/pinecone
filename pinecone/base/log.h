//
// Created by 28192 on 2024/7/31.
//

#ifndef PINECONE_PINECONE_BASE_LOG_H
#define PINECONE_PINECONE_BASE_LOG_H

#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <pinecone/util/singleton.h>
#include <pinecone/util/util.h>

#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <ctime>

#include <source_location>
#include <format>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <chrono>


#define PINECONE_LOG_LEVEL(logger, level)                                                        \
  if (logger->GetLevel() <= level)                                                               \
  pinecone::base::LogEventWrap(pinecone::base::LogEvent::ptr(new pinecone::base::LogEvent(       \
                                   logger, level, 0, pinecone::util::GetThreadId(), 0, "main"))) \
      .GetSS()

#define PINECONE_LOG_TRACE(logger)    PINECONE_LOG_LEVEL(logger, pinecone::base::LogLevel::kTrace)
#define PINECONE_LOG_DEBUG(logger)    PINECONE_LOG_LEVEL(logger, pinecone::base::LogLevel::kDebug)
#define PINECONE_LOG_INFO(logger)     PINECONE_LOG_LEVEL(logger, pinecone::base::LogLevel::kInfo)
#define PINECONE_LOG_CRITICAL(logger) PINECONE_LOG_LEVEL(logger, pinecone::base::LogLevel::kCritical)
#define PINECONE_LOG_WARN(logger)     PINECONE_LOG_LEVEL(logger, pinecone::base::LogLevel::kWarn)
#define PINECONE_LOG_ERROR(logger)    PINECONE_LOG_LEVEL(logger, pinecone::base::LogLevel::kError)
#define PINECONE_LOG_FATAL(logger)    PINECONE_LOG_LEVEL(logger, pinecone::base::LogLevel::kFatal)

#define PINECONE_LOG_ROOT()           pinecone::base::LoggerMgr::Instance()->GetRoot()
#define PINECONE_LOG_NAME(name)       pinecone::base::LoggerMgr::Instance()->GetLogger(name)

#define PINECONE_LOG_FMT_LEVEL(logger, level, fmt, ...)                                              \
  if (logger->GetLevel() <= level)                                                                   \
  pinecone::base::LogEventWrap(pinecone::base::LogEvent::ptr(new pinecone::base::LogEvent(           \
                                   logger, level, 0, pinecone::util::GetThreadId(), 0, "fmt_main"))) \
      .GetEvent()                                                                                    \
      ->Format(fmt, __VA_ARGS__)

#define PINECONE_LOG_FMT_TRACE(logger, fmt, ...) \
  PINECONE_LOG_FMT_LEVEL(logger, pinecone::base::LogLevel::kTrace, fmt, __VA_ARGS__)
#define PINECONE_LOG_FMT_DEBUG(logger, fmt, ...) \
  PINECONE_LOG_FMT_LEVEL(logger, pinecone::base::LogLevel::kDebug, fmt, __VA_ARGS__)
#define PINECONE_LOG_FMT_INFO(logger, fmt, ...) \
  PINECONE_LOG_FMT_LEVEL(logger, pinecone::base::LogLevel::kInfo, fmt, __VA_ARGS__)
#define PINECONE_LOG_FMT_CRITICAL(logger, fmt, ...) \
  PINECONE_LOG_FMT_LEVEL(logger, pinecone::base::LogLevel::kCritical, fmt, __VA_ARGS__)
#define PINECONE_LOG_FMT_WARN(logger, fmt, ...) \
  PINECONE_LOG_FMT_LEVEL(logger, pinecone::base::LogLevel::kWarn, fmt, __VA_ARGS__)
#define PINECONE_LOG_FMT_ERROR(logger, fmt, ...) \
  PINECONE_LOG_FMT_LEVEL(logger, pinecone::base::LogLevel::kError, fmt, __VA_ARGS__)
#define PINECONE_LOG_FMT_FATAL(logger, fmt, ...) \
  PINECONE_LOG_FMT_LEVEL(logger, pinecone::base::LogLevel::kFatal, fmt, __VA_ARGS__)

namespace pinecone::base {
class Logger;

class LoggerManager;

enum class LogLevel : std::uint8_t {
  kUnknow [[maybe_unused]],
  kTrace [[maybe_unused]],
  kDebug [[maybe_unused]],
  kInfo [[maybe_unused]],
  kCritical [[maybe_unused]],
  kWarn [[maybe_unused]],
  kError [[maybe_unused]],
  kFatal [[maybe_unused]],
};

class LogEvent {
 public:
  using ptr = std::shared_ptr<LogEvent>;

  LogEvent(std::shared_ptr<Logger> logger, LogLevel level, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id,
           std::string_view thread_name, std::source_location loc = std::source_location::current());

  [[maybe_unused]] auto GetFilename() const -> std::string_view {
    return (strrchr(source_location_.file_name(), '/') ? (strrchr(source_location_.file_name(), '/') + 1)
                                                       : source_location_.file_name());
  }

  [[maybe_unused]] auto GetLine() const -> uint32_t { return source_location_.line(); }

  [[maybe_unused]] auto GetFunctionName() const -> std::string_view { return source_location_.function_name(); }

  [[maybe_unused]] auto GetElapse() const -> uint32_t { return elapse_; }

  [[maybe_unused]] auto GetThreadID() const -> uint32_t { return thread_id_; }

  [[maybe_unused]] auto GetFiberID() const -> uint32_t { return fiber_id_; }

  [[maybe_unused]] auto GetTime() const -> std::chrono::system_clock::time_point { return time_; }

  [[maybe_unused]] auto GetThreadName() const -> std::string_view { return thread_name_; }

  [[maybe_unused]] auto GetContent() const -> std::string { return ss_.str(); }

  [[maybe_unused]] auto GetLogger() const -> std::shared_ptr<Logger> { return logger_; }

  [[maybe_unused]] auto GetLevel() const -> LogLevel { return level_; }

  [[maybe_unused]] auto GetSS() -> std::stringstream & { return ss_; }

  //  [[maybe_unused]] auto Format(std::string_view fmt, ...) -> void;
  //  [[maybe_unused]] auto Format(std::string_view fmt, va_list vlt) -> void;

  template<typename... Args>
  [[maybe_unused]] auto Format(std::format_string<Args...> fmt, Args &&... args) -> void {
    ss_ << std::vformat(fmt.get(), std::make_format_args(args...));
  }

 private:
  uint32_t elapse_{0};
  uint32_t thread_id_{0};
  uint32_t fiber_id_{0};
  LogLevel level_{LogLevel::kUnknow};
  std::source_location source_location_;
  std::chrono::system_clock::time_point time_{std::chrono::high_resolution_clock::now()};
  std::string thread_name_;
  std::stringstream ss_;
  std::shared_ptr<Logger> logger_;
};

class LogEventWrap {
 public:
  explicit LogEventWrap(LogEvent::ptr event) : event_(event) {}

  ~LogEventWrap();

  [[nodiscard]] [[maybe_unused]] auto GetEvent() const -> LogEvent::ptr { return event_; }

  [[nodiscard]] [[maybe_unused]] auto GetSS() -> std::stringstream & { return event_->GetSS(); }

 private:
  LogEvent::ptr event_;
};


class LogFormatter {
 public:
  using ptr = std::shared_ptr<LogFormatter>;

  class FormatItem {
   public:
    using ptr = std::shared_ptr<FormatItem>;

    virtual ~FormatItem() = default;

    virtual auto Format(std::ostream &ost, std::shared_ptr<Logger> logger, LogLevel level,
                        LogEvent::ptr event) -> void = 0;
  };

  explicit LogFormatter(std::string_view pattern) : pattern_(pattern) { Init(); }

  auto Format(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> std::string;

  auto Format(std::ostream &ofs, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> std::ostream &;

  [[maybe_unused]] [[nodiscard]] auto IsError() const -> bool { return error_; }

  [[maybe_unused]] [[nodiscard]] auto GetPattern() const -> std::string_view { return pattern_; }

 private:
  auto Init() -> void;

  std::string pattern_;
  std::vector<FormatItem::ptr> items_;
  bool error_{false};
};

class LogAppender {
  friend class Logger;

 public:
  using ptr = std::shared_ptr<LogAppender>;

  virtual ~LogAppender() = default;

  virtual auto Log(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void = 0;

  virtual auto ToJsonString() -> std::string = 0;

  auto SetFormatter(LogFormatter::ptr formatter) -> void {
    formatter_ = formatter;
    has_formatter_ = static_cast<bool>(formatter_);
  }

  auto GetFormatter() -> LogFormatter::ptr { return formatter_; }

  auto SetLevel(LogLevel level) -> void { level_ = level; }

  auto GetLevel() const -> LogLevel { return level_; }

 protected:
  LogLevel level_{LogLevel::kTrace};
  bool has_formatter_{false};
  LogFormatter::ptr formatter_;
};

class StdoutLogAppender : public LogAppender {
 public:
  using ptr = std::shared_ptr<StdoutLogAppender>;

  auto Log(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    if (level < level_) {
      return;
    }
    formatter_->Format(std::cerr, logger, level, event);
  };

  auto ToJsonString() -> std::string override { return nlohmann::json{}.dump(); };
};

class FileLogAppender : public LogAppender {
 public:
  using ptr = std::shared_ptr<FileLogAppender>;

  explicit FileLogAppender(std::string_view filename) : filename_(filename) {
    filestream_.open(filename.data(), std::ios::app);
    if (!filestream_) {
      std::cout << "open " << filename_ << " failed\n";
      return;
    }
  }

  auto Log(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    if (level < level_)
      return;

    if (!formatter_->Format(filestream_, logger, level, event)) {
      std::cerr << "file log error\n";
    }
  }

  auto ToJsonString() -> std::string override { return nlohmann::json{}.dump(); };

 private:
  std::string filename_;
  std::ofstream filestream_;
};

class Logger : public std::enable_shared_from_this<Logger> {
  friend class LoggerManager;

 public:
  using ptr = std::shared_ptr<Logger>;

  explicit Logger(std::string_view name = "root");

  auto Log(LogLevel level, LogEvent::ptr event) -> void;

  [[maybe_unused]] auto Trace(const LogEvent::ptr &event) -> void { Log(LogLevel::kTrace, event); }

  [[maybe_unused]] auto Debug(const LogEvent::ptr &event) -> void { Log(LogLevel::kDebug, event); }

  [[maybe_unused]] auto Info(const LogEvent::ptr &event) -> void { Log(LogLevel::kInfo, event); }

  [[maybe_unused]] auto Critical(const LogEvent::ptr &event) -> void { Log(LogLevel::kWarn, event); }

  [[maybe_unused]] auto Warn(const LogEvent::ptr &event) -> void { Log(LogLevel::kCritical, event); }

  [[maybe_unused]] auto Error(const LogEvent::ptr &event) -> void { Log(LogLevel::kError, event); }

  [[maybe_unused]] auto Fatal(const LogEvent::ptr &event) -> void { Log(LogLevel::kFatal, event); }

  [[maybe_unused]] auto AddAppender(LogAppender::ptr appender) -> void;

  [[maybe_unused]] auto DelAppender(LogAppender::ptr appender) -> void;

  [[maybe_unused]] auto ClearAppender() -> void { appenders_.clear(); }

  [[maybe_unused]] auto SetLevel(LogLevel level) { level_ = level; }

  [[maybe_unused]] auto GetLevel() const -> LogLevel { return level_; }

  [[maybe_unused]] auto GetName() const -> std::string_view { return name_; }

  [[maybe_unused]] auto SetFormatter(LogFormatter::ptr fmt) -> void;

  [[maybe_unused]] auto SetFormatter(std::string_view fmt) -> void;

  [[maybe_unused]] auto GetFormatter() -> LogFormatter::ptr { return formatter_; }

  [[maybe_unused]] auto ToJsonString() -> std::string { return nlohmann::json{}.dump(); }

 private:
  std::string name_;
  LogLevel level_{LogLevel::kTrace};
  std::list<LogAppender::ptr> appenders_;
  LogFormatter::ptr formatter_{
      std::make_shared<LogFormatter>("%d{%Y-%m-%d %H:%M:%S} %t %N %F [%p] [%c] %m -> %f:%l%n")};
  Logger::ptr root_;
};

class LoggerManager {
 public:
  LoggerManager();

  auto GetLogger(std::string_view name) -> Logger::ptr;

  auto GetRoot() -> Logger::ptr { return root_; }

  auto Init() -> void {};

  [[maybe_unused]] auto ToJsonString() -> std::string {
    auto json = nlohmann::json{};
    return json.dump();
  }

 private:
  std::unordered_map<std::string, Logger::ptr> loggers_;
  Logger::ptr root_;
};

using LoggerMgr = pinecone::util::Singleton<LoggerManager>;

}


#endif //PINECONE_PINECONE_BASE_LOG_H
