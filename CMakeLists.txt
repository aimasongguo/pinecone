cmake_minimum_required(VERSION 3.30)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -Wall -Wno-deprecated -Werror -Wno-old-style-cast -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -Wall -Wno-deprecated -Werror -Wno-old-style-cast -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(pinecone VERSION 0.0.1 LANGUAGES CXX)

add_subdirectory(pinecone)
