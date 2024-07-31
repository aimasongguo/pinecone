//
// Created by 28192 on 2024/7/31.
//

#include <pinecone/base/config.h>
#include <iostream>

namespace pinecone::base {

namespace {

auto ListAllMember(std::string_view prefix, const nlohmann::json &obj,
                   std::unordered_map<std::string, nlohmann::json> &output) {
  if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
    LOG(FATAL) << "Config invalid name : " << prefix << " : " << obj;
    return;
  }

  output.insert(std::make_pair(prefix, obj));

  if (obj.is_object()) {
    for (const auto &item : obj.items()) {
      ListAllMember(prefix.empty() ? item.key() : std::string(prefix.data()) + "." + item.key(), item.value(), output);
    }
  }
}

}

auto Config::LoadFromJson(const nlohmann::json &root) -> void {
  std::unordered_map<std::string, nlohmann::json> all_obj;
  ListAllMember("", root, all_obj);

  for (const auto &item : all_obj) {
    auto key = item.first;
    if (key.empty()) {
      continue;
    }

    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    auto var = LookupBase(key);
    if (var) {
      var->FromJson(item.second);
    }
  }
}

auto Config::LookupBase(std::string_view name) -> ConfigVarBase::ptr {
  auto item = GetDatas().find(name.data());
  return item == GetDatas().end() ? nullptr : item->second;
}

}  // namespace pinecone