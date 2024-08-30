include(${CMAKE_SOURCE_DIR}/cmake/pinecone.cmake)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

set(PCSG_BASE_LINK_LIBS rt dl pthread)

add_library(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

file(GLOB_RECURSE ALL_PCSG_LIB_HEADER_FILES
        "./*.h"
)

file(GLOB_RECURSE ALL_PCSG_LIB_SOURCE_FILES
        "./*.cc"
)

target_sources(${PROJECT_NAME} PRIVATE ${ALL_PCSG_LIB_SOURCE_FILES} ${ALL_PCSG_LIB_HEADER_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}")
target_link_libraries(${PROJECT_NAME} PRIVATE ${PCSG_BASE_LINK_LIBS})
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_compile_definitions(${PROJECT_NAME} PRIVATE GIT_VERSION="${GIT_VERSION}")
target_compile_definitions(${PROJECT_NAME} PRIVATE GIT_DATE="${GIT_DATE}")
target_compile_definitions(${PROJECT_NAME} PRIVATE GIT_HASH="${GIT_HASH}")
target_compile_definitions(${PROJECT_NAME} PRIVATE BUILD_TIME="${BUILD_TIME}")
