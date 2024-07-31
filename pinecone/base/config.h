//
// Created by 28192 on 2024/7/31.
//

#ifndef PINECONE_PINECONE_BASE_CONFIG_H
#define PINECONE_PINECONE_BASE_CONFIG_H


#include <glog/logging.h>

#include <boost/lexical_cast.hpp>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace pinecone::base {

class ConfigVarBase {
 public:
  using ptr = std::shared_ptr<ConfigVarBase>;

  explicit ConfigVarBase(std::string_view name, std::string_view description = "")
      : name_(name), description_(description) {
    std::transform(name_.begin(), name_.end(), name_.begin(), ::tolower);
  }

  virtual ~ConfigVarBase() = default;

  [[maybe_unused]] [[nodiscard]] auto GetName() const -> std::string_view { return name_; }

  [[maybe_unused]] [[nodiscard]] auto GetDescription() const -> std::string_view { return description_; }

  virtual auto ToString() -> std::string = 0;

  virtual auto FromJson(const nlohmann::json &val) -> bool = 0;

 protected:
  std::string name_;
  std::string description_;
};

template<class T>
class ConfigVar : public ConfigVarBase {
 public:
  using ptr = std::shared_ptr<ConfigVar>;
  using on_change_cb = std::function<void(const T &old_value, const T &new_value)>;

  ConfigVar(std::string_view name, const T &default_value, std::string_view description = "")
      : ConfigVarBase(name, description), val_(default_value) {}

  auto ToString() -> std::string override {
    try {
      //      return boost::lexical_cast<std::string>(val_);
      nlohmann::json json = val_;
      return json.dump();
    } catch (std::exception &e) {
      LOG(FATAL) << "ConfigVar::fromString exception " << e.what() << " convert: string to" << typeid(val_).name()
                 << " name=" << name_;
    }
    return "";
  };

  auto FromJson(const nlohmann::json &val) -> bool override {
    try {
      SetValue(val.get<T>());
    } catch (std::exception &e) {
      LOG(FATAL) << "ConfigVar::fromString exception " << e.what() << " convert: string to \"" << typeid(val_).name()
                 << "\" name=" << name_ << " - " << val.dump();
    }
    return false;
  };

  auto GetValue() const -> T { return val_; }

  auto SetValue(const T &value) { val_ = value; }

  auto AddListener(on_change_cb cb) -> uint64_t {
    static uint64_t s_fun_id{0};
    ++s_fun_id;
    cbs_[s_fun_id] = cb;
    return s_fun_id;
  }

  auto DelListener(uint64_t key) -> void {
    cbs_.erase(key);
  }

  auto GetListener(uint64_t key) -> on_change_cb {
    auto item = cbs_.find(key);
    return item == cbs_.end() ? nullptr : item->second;
  }

  auto ClearListener() -> void {
    cbs_.clear();
  }

 private:
  T val_;
  std::map<uint64_t, on_change_cb> cbs_;
};

class Config {
 public:
  using ConfigVarMap = std::unordered_map<std::string, ConfigVarBase::ptr>;

  template<class T>
  static auto Lookup(std::string_view name, const T &default_value, std::string_view description = "") ->
  typename ConfigVar<T>::ptr {
    auto tmp = Lookup<T>(name);
    if (tmp) {
      LOG(ERROR) << "Lookup name=" << name << " exists";
      return tmp;
    }
    if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
      LOG(ERROR) << "Lookup name invalid ";
      throw std::invalid_argument(name.data());
    }

    auto var_base = std::make_shared<ConfigVar<T>>(name, default_value, description);
    GetDatas()[name.data()] = var_base;
    return var_base;
  }

  template<class T>
  static auto Lookup(std::string_view name) -> typename ConfigVar<T>::ptr {
    auto item = GetDatas().find(name.data());
    if (item == GetDatas().end()) {
      return nullptr;
    }

    return std::dynamic_pointer_cast<ConfigVar<T>>(item->second);
  }

  static auto LoadFromJson(const nlohmann::json &root) -> void;

  static auto LookupBase(std::string_view name) -> ConfigVarBase::ptr;

 private:
  static auto GetDatas() -> ConfigVarMap & {
    static ConfigVarMap s_datas;
    return s_datas;
  }
};

};  // namespace pinecone::base

#endif //PINECONE_PINECONE_BASE_CONFIG_H
