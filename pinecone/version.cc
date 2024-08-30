//
// Created by 28192 on 2024/8/30.
//

#include <pinecone/version.h>

namespace pinecone {
namespace {
constexpr std::string_view kMiniVersion{PROJECT_VERSION};
constexpr std::string_view kGitVersion{GIT_VERSION};
constexpr std::string_view kGitData{GIT_DATE};
constexpr std::string_view kGitHash{GIT_HASH};
constexpr std::string_view kBuildTime{BUILD_TIME};
constexpr std::string_view kDetailVersion{
    PROJECT_VERSION "." BUILD_TIME "-" GIT_VERSION "-" GIT_DATE "-" GIT_HASH
};
}

[[maybe_unused]] auto get_build_time() -> std::string_view {
  return kBuildTime;
}

[[maybe_unused]] auto get_mini_version() -> std::string_view {
  return kMiniVersion;
}

[[maybe_unused]] auto get_detail_version() -> std::string_view {
  return kDetailVersion;
}

[[maybe_unused]] auto get_git_version() -> std::string_view {
  return kGitVersion;
}

[[maybe_unused]] auto get_git_date() -> std::string_view {
  return kGitData;
}

[[maybe_unused]] auto get_git_hash() -> std::string_view {
  return kGitHash;
}
}
