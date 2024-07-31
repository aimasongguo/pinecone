//
// Created by 28192 on 2024/7/31.
//

#include "util.h"

#include <syscall.h>
#include <unistd.h>

namespace pinecone::util {

auto GetThreadId() -> pid_t {
  static thread_local auto t_pid = syscall(SYS_gettid);
  return static_cast<int>(t_pid);
}

}