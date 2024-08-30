//
// Created by 28192 on 2024/8/30.
//

#ifndef PINECONE_PINECONE_VERSION_H
#define PINECONE_PINECONE_VERSION_H

#include <string_view>

namespace pinecone {

[[maybe_unused]] auto get_build_time() -> std::string_view;
[[maybe_unused]] auto get_mini_version() -> std::string_view;
[[maybe_unused]] auto get_detail_version() -> std::string_view;
[[maybe_unused]] auto get_git_version() -> std::string_view;
[[maybe_unused]] auto get_git_date() -> std::string_view;
[[maybe_unused]] auto get_git_hash() -> std::string_view;

}

#endif //PINECONE_PINECONE_VERSION_H
