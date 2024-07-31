//
// Created by 28192 on 2024/7/31.
//

#ifndef PINECONE_PINECONE_UTIL_SINGLETON_H
#define PINECONE_PINECONE_UTIL_SINGLETON_H

#include <memory>

namespace pinecone::util {

template <class T>
class Singleton {
 public:
  static auto Instance() -> T* {
    static T s_class;
    return &s_class;
  }
};

template <class T>
class SingletonPtr {
 public:
  static auto Instance() -> std::shared_ptr<T> {
    static auto s_ptr(std::make_shared<T>());
    return s_ptr;
  }
};

}  // namespace pinecone::util

#endif  // PINECONE_PINECONE_UTIL_SINGLETON_H
