set(PINECONE_BASE_LIBS OpenSSL fmt Boost nlohmann_json gRPC glog magic_enum fmt Libevent benchmark CLI11)
foreach (BASE_LIBS ${PINECONE_BASE_LIBS})
    find_package(${BASE_LIBS} REQUIRED)
endforeach ()

set(PINECONE_BASE_LINK_LIBS rt dl pthread openssl::openssl fmt::fmt boost::boost nlohmann_json::nlohmann_json grpc::grpc glog::glog magic_enum::magic_enum fmt::fmt CLI11::CLI11 libevent::libevent benchmark::benchmark_main)

add_library(${PROJECT_NAME} SHARED)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

file(GLOB_RECURSE ALL_PINECONE_LIB_HEADER_FILES
        "base/*.h"
        "util/*.h"
)

file(GLOB_RECURSE ALL_PINECONE_LIB_SOURCE_FILES
        "base/*.cc"
        "util/*.cc"
)

target_sources(${PROJECT_NAME} PRIVATE ${ALL_PINECONE_LIB_SOURCE_FILES} ${ALL_PINECONE_LIB_HEADER_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} ${PINECONE_BASE_LINK_LIBS})
target_compile_definitions(${PROJECT_NAME} PUBLIC PROJECT_VERSION="${PROJECT_VERSION}")


