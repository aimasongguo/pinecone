//
// Created by 28192 on 2024/7/31.
//

#include <pinecone/base/log.h>
#include <format>
#include <iomanip>


namespace pinecone::base {

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel level, uint32_t elapse, uint32_t thread_id,
                   uint32_t fiber_id, std::string_view thread_name, std::source_location loc)
    : elapse_(elapse),
      thread_id_(thread_id),
      fiber_id_(fiber_id),
      level_(level),
      source_location_(loc),
      thread_name_(thread_name),
      logger_(logger) {}

// auto LogEvent::Format(std::string_view fmt, ...) -> void {
//  va_list vlt{};
//  va_start(vlt, fmt);
//  Format(fmt, vlt);
//  va_end(vlt);
//}

// auto LogEvent::Format(std::string_view fmt, va_list vlt) -> void {
//  char* buf{nullptr};
//  auto len = vasprintf(&buf, fmt.data(), vlt);
//  if (len == -1) {
//    std::cerr << "vasprintf failed\n";
//    return;
//  }
//  ss_ << std::string(buf, len);
//  free(buf);
//}

LogEventWrap::~LogEventWrap() { event_->GetLogger()->Log(event_->GetLevel(), event_); }

class MessageFormatItem : public LogFormatter::FormatItem {
 public:
  explicit MessageFormatItem(const std::string &str = "") {}

  auto Format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    os << event->GetContent();
  }
};

class LevelFormatItem : public LogFormatter::FormatItem {
 public:
  explicit LevelFormatItem(const std::string &str = "") {}

  auto Format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    os << magic_enum::enum_name(level);
  }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
 public:
  explicit ElapseFormatItem(const std::string &str = "") {}

  auto Format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    os << event->GetElapse();
  }
};

class NameFormatItem : public LogFormatter::FormatItem {
 public:
  explicit NameFormatItem(const std::string &str = "") {}

  auto Format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    os << event->GetLogger()->GetName();
  }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
 public:
  explicit ThreadIdFormatItem(const std::string &str = "") {}

  auto Format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    os << event->GetThreadID();
  }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
 public:
  explicit FiberIdFormatItem(const std::string &str = "") {}

  auto Format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    os << event->GetFiberID();
  }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
 public:
  explicit ThreadNameFormatItem(const std::string &str = "") {}

  auto Format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    os << event->GetThreadName();
  }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
 public:
  explicit DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S") : format_(format) {
    if (format_.empty()) {
      format_ = "%Y-%m-%d %H:%M:%S";
    }
  }

  auto Format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    os << std::format("{}",
                      std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::high_resolution_clock::now()});
  }

 private:
  std::string format_;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
 public:
  explicit FilenameFormatItem(const std::string &str = "") {}

  auto Format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    os << event->GetFilename();
  }
};

class LineFormatItem : public LogFormatter::FormatItem {
 public:
  explicit LineFormatItem(const std::string &str = "") {}

  auto Format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    os << event->GetLine();
  }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
 public:
  explicit NewLineFormatItem(const std::string &str = "") {}

  auto Format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    os << '\n';
  }
};

class StringFormatItem : public LogFormatter::FormatItem {
 public:
  explicit StringFormatItem(const std::string &str) : string_(str) {}

  auto Format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    os << string_;
  }

 private:
  std::string string_;
};

class TabFormatItem : public LogFormatter::FormatItem {
 public:
  explicit TabFormatItem(const std::string &str = "") {}

  auto Format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> void override {
    os << "\t";
  }
};

auto LogFormatter::Init() -> void {
  // str, format, type
  std::vector<std::tuple<std::string, std::string, int>> vec;
  std::string nstr;
  for (size_t i = 0; i < pattern_.size(); ++i) {
    if (pattern_[i] != '%') {
      nstr.append(1, pattern_[i]);
      continue;
    }

    if ((i + 1) < pattern_.size()) {
      if (pattern_[i + 1] == '%') {
        nstr.append(1, '%');
        continue;
      }
    }

    size_t n = i + 1;
    int fmt_status = 0;
    size_t fmt_begin = 0;

    std::string str;
    std::string fmt;
    while (n < pattern_.size()) {
      if (!fmt_status && (!isalpha(pattern_[n]) && pattern_[n] != '{' && pattern_[n] != '}')) {
        str = pattern_.substr(i + 1, n - i - 1);
        break;
      }
      if (fmt_status == 0) {
        if (pattern_[n] == '{') {
          str = pattern_.substr(i + 1, n - i - 1);
          // std::cout << "*" << str << std::endl;
          fmt_status = 1;  // 解析格式
          fmt_begin = n;
          ++n;
          continue;
        }
      } else if (fmt_status == 1) {
        if (pattern_[n] == '}') {
          fmt = pattern_.substr(fmt_begin + 1, n - fmt_begin - 1);
          // std::cout << "#" << fmt << std::endl;
          fmt_status = 0;
          ++n;
          break;
        }
      }
      ++n;
      if (n == pattern_.size()) {
        if (str.empty()) {
          str = pattern_.substr(i + 1);
        }
      }
    }

    if (fmt_status == 0) {
      if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, std::string(), 0));
        nstr.clear();
      }
      vec.push_back(std::make_tuple(str, fmt, 1));
      i = n - 1;
    } else if (fmt_status == 1) {
      std::cout << "pattern parse error: " << pattern_ << " - " << pattern_.substr(i) << '\n';
      error_ = true;
      vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
    }
  }

  if (!nstr.empty()) {
    vec.push_back(std::make_tuple(nstr, "", 0));
  }
  static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {
#define XX(str, C)                                                           \
  {                                                                          \
    #str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt)); } \
  }

      XX(m, MessageFormatItem),     // m:消息
      XX(p, LevelFormatItem),       // p:日志级别
      XX(r, ElapseFormatItem),      // r:累计毫秒数
      XX(c, NameFormatItem),        // c:日志名称
      XX(t, ThreadIdFormatItem),    // t:线程id
      XX(n, NewLineFormatItem),     // n:换行
      XX(d, DateTimeFormatItem),    // d:时间
      XX(f, FilenameFormatItem),    // f:文件名
      XX(l, LineFormatItem),        // l:行号
      XX(T, TabFormatItem),         // T:Tab
      XX(F, FiberIdFormatItem),     // F:协程id
      XX(N, ThreadNameFormatItem),  // N:线程名称
#undef XX
  };

  for (auto &i : vec) {
    if (std::get<2>(i) == 0) {
      items_.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
    } else {
      auto it = s_format_items.find(std::get<0>(i));
      if (it == s_format_items.end()) {
        items_.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
        error_ = true;
      } else {
        items_.push_back(it->second(std::get<1>(i)));
      }
    }

    // std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ")
    // - (" << std::get<2>(i) << ")" << std::endl;
  }
  // std::cout << m_items.size() << std::endl;
}

auto LogFormatter::Format(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) -> std::string {
  std::stringstream ss;
  for (auto &item : items_) {
    item->Format(ss, logger, level, event);
  }
  return ss.str();
}

auto LogFormatter::Format(std::ostream &ofs, std::shared_ptr<Logger> logger, LogLevel level,
                          LogEvent::ptr event) -> std::ostream & {
  std::stringstream ss;
  for (auto &item : items_) {
    item->Format(ss, logger, level, event);
  }
  return ofs << ss.str();
}

auto Logger::Log(LogLevel level, LogEvent::ptr event) -> void {
  if (level < level_) {
    return;
  }

  auto self = shared_from_this();
  if (!appenders_.empty()) {
    for (auto &item : appenders_) {
      item->Log(self, level, event);
    }
  } else if (root_) {
    root_->Log(level, event);
  }
}

auto Logger::AddAppender(LogAppender::ptr appender) -> void {
  if (!appender->GetFormatter()) {
    appender->formatter_ = formatter_;
  }
  appenders_.push_back(appender);
}

auto Logger::DelAppender(LogAppender::ptr appender) -> void {
  for (auto item = appenders_.begin(); item != appenders_.end(); item++) {
    if (*item == appender) {
      appenders_.erase(item);
    }
  }
}

auto Logger::SetFormatter(LogFormatter::ptr fmt) -> void {
  formatter_ = fmt;
  for (auto &item : appenders_) {
    if (!item->has_formatter_) {
      item->formatter_ = fmt;
    }
  }
}

auto Logger::SetFormatter(std::string_view fmt) -> void {
  std::cout << "Logger::setFormatter -> [" << fmt << "]\n";
  auto new_fmt = std::make_shared<LogFormatter>(fmt);
  if (new_fmt->IsError()) {
    std::cout << "Logger setFormatter name=" << name_ << " value=" << fmt << " invalid formatter.\n";
    return;
  }
  SetFormatter(new_fmt);
}

Logger::Logger(std::string_view name) : name_(name) {}

LoggerManager::LoggerManager() {
  root_ = std::make_shared<Logger>();
  root_->AddAppender(std::make_shared<StdoutLogAppender>());
  loggers_[root_->name_] = root_;
  Init();
}

auto LoggerManager::GetLogger(std::string_view name) -> Logger::ptr {
  auto item = loggers_.find(name.data());
  if (item != loggers_.end()) {
    return item->second;
  }
  auto logger = std::make_shared<Logger>(name);
  logger->root_ = root_;
  loggers_[name.data()] = logger;
  return logger;
}


}  // namespace pinecone::base