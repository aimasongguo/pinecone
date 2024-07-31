//
// Created by 28192 on 2024/7/31.
//
#include <pinecone/base/log.h>

#include <memory>

auto main() -> int {

  PINECONE_LOG_TRACE(PINECONE_LOG_NAME("system")) << "warn log";

  PINECONE_LOG_FMT_DEBUG(PINECONE_LOG_NAME("fmt"), "{} {}", "this fmt log", 42);

  return 0;
}