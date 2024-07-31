//
// Created by 28192 on 2024/7/31.
//
#include <glog/logging.h>
#include <pinecone/base/config.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>

constexpr auto kSystemPort{8080};
constexpr auto kSystemFloatPort{99.88F};

struct Server {
  std::string ip;
  int port;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Server, ip, port)
};

auto g_int_value_config =
    pinecone::base::Config::Lookup<int>("system.port", kSystemPort, "system port");

auto g_float_value_config =
    pinecone::base::Config::Lookup<float>("system.float", kSystemFloatPort, "system float port");

auto g_int_vec_value_config =
    pinecone::base::Config::Lookup<std::vector<int>>("system.int_vec", std::vector<int>{1, 2, 3}, "system int vec");

auto g_double_list_value_config =
    pinecone::base::Config::Lookup<std::list<double>>("system.double_list", std::list<double>{1.1, 2.2, 3.3}, "system double list");

auto g_float_set_value_config =
    pinecone::base::Config::Lookup<std::set<float>>("system.float_set", std::set<float>{1.1, 2.2, 3.3}, "system float set");

auto g_server_value_config =
    pinecone::base::Config::Lookup<Server>("system.server", Server{"192.168.61.21", 9999}, "system server");

auto g_map_server_value_config =
    pinecone::base::Config::Lookup<std::map<std::string, Server>>("system.map_server", std::map<std::string, Server>{
        std::make_pair("server_1", Server{"1.1.1.1", 11}),
        std::make_pair("server_2", Server{"2.2.2.2", 22})
    }, "system map server");

auto g_root_log_level_value_config =
    pinecone::base::Config::Lookup<int>("root.log.level", 0, "root log level");

auto PrintJsonToTerminal(const nlohmann::json& j, int level = 1) -> void {
  // 如果JSON是一个对象，我们遍历其所有键值对
  if (j.is_object()) {
    for (const auto& elem : j.items()) {
      std::cout << elem.key() << ": ";

      // 根据值的类型进行不同的处理
      if (elem.value().is_string()) {
        std::cout << "\"" << elem.value() << "\"";
      } else if (elem.value().is_number_integer() || elem.value().is_number_float()) {
        std::cout << elem.value();
      } else if (elem.value().is_boolean()) {
        std::cout << (elem.value() ? "true" : "false");
      } else if (elem.value().is_array()) {
        std::cout << "[";
        bool first = true;
        for (const auto& val : elem.value()) {
          if (!first) {
            std::cout << ", ";
          }
          PrintJsonToTerminal(val, level + 1); // 递归处理数组中的元素
          first = false;
        }
        std::cout << "]";
      } else if (elem.value().is_object()) {
        std::cout << "{\n";
        PrintJsonToTerminal(elem.value(), level + 1); // 递归处理对象中的元素
        std::cout << "}\n";
      } else {
        // 对于其他类型，我们可以简单地输出为字符串（或者根据需要自定义）
        std::cout << "null"; // 注意：这里应该是更具体的处理，但为简单起见，我们假设未知类型输出为null
      }

      std::cout << '\n';
    }
  } else {
    // 如果JSON不是一个对象（例如，它是一个数组、字符串、数字等），我们可以根据需要进行处理
    // 但在这个例子中，我们假设只处理对象
    std::cout << "JSON is not an object" << '\n';
  }
}

auto TestJson() {
  const auto cfp{std::filesystem::path{"../config/config/config.json"}};
  std::ifstream ifs(cfp);
  if (!ifs.is_open()) {
    throw std::runtime_error(std::string("Filed to open ").append(cfp));
  }

  nlohmann::json json;
  ifs >> json;

  LOG(INFO) << json;

  PrintJsonToTerminal(json);
}

auto main([[maybe_unused]] int argc,[[maybe_unused]] char** argv) -> int {
  google::InitGoogleLogging(argv[0]);
  // 设置日志输出到stderr，而不是默认的文件
  FLAGS_logtostderr = true;

  // 设置日志级别为INFO（默认是INFO，但这里明确设置）
  FLAGS_stderrthreshold = 0;  // 0 = INFO, 1 = WARNING, 2 = ERROR

  LOG(INFO) << "system port = " << g_int_value_config->GetValue();
  LOG(INFO) << "system float port = " << g_float_value_config->GetValue();
  LOG(INFO) << "root log level = " << g_root_log_level_value_config->GetValue();

  for (auto & item : g_int_vec_value_config->GetValue()) {
    LOG(INFO) << "int vec : " << item;
  }

  for (auto & item : g_double_list_value_config->GetValue()) {
    LOG(INFO) << "double list : " << item;
  }

  for (const auto & item : g_float_set_value_config->GetValue()) {
    LOG(INFO) << "float set: " << item;
  }

  LOG(INFO) << "system server ip : \"" << g_server_value_config->GetValue().ip << ", port = " << g_server_value_config->GetValue().port;

  for (const auto & [name, server] : g_map_server_value_config->GetValue()) {
    LOG(INFO) << "system server name:\"" << name << "\", ip : \"" << server.ip << "\", port = " << server.port;
  }

  const auto cfp{std::filesystem::path{"../config/config/config.json"}};
  std::ifstream ifs(cfp);
  if (!ifs.is_open()) {
    throw std::runtime_error(std::string("Filed to open ").append(cfp));
  }

  nlohmann::json json;
  ifs >> json;
  pinecone::base::Config::LoadFromJson(json);

  LOG(INFO) << "system port = " << g_int_value_config->GetValue();
  LOG(INFO) << "system float port = " << g_float_value_config->GetValue();
  LOG(INFO) << "root log level = " << g_root_log_level_value_config->GetValue();
  for (auto & item : g_int_vec_value_config->GetValue()) {
    LOG(INFO) << "int vec : " << item;
  }

  for (auto & item : g_double_list_value_config->GetValue()) {
    LOG(INFO) << "double list : " << item;
  }

  for (const auto & item : g_float_set_value_config->GetValue()) {
    LOG(INFO) << "float set: " << item;
  }

  LOG(INFO) << "system server ip : \"" << g_server_value_config->GetValue().ip << ", port = " << g_server_value_config->GetValue().port;

  for (const auto & [name, server] : g_map_server_value_config->GetValue()) {
    LOG(INFO) << "system server name:\"" << name << "\", ip : \"" << server.ip << "\", port = " << server.port;
  }

  google::ShutdownGoogleLogging();
  return 0;
}