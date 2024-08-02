include_directories(${CMAKE_SOURCE_DIR})

file(GLOB_RECURSE ALL_PINECONE_TEST_FILES "./*.cc")

set(ALL_PINECONE_TEST_BASE_LIBS  OpenSSL fmt Boost nlohmann_json gRPC glog magic_enum fmt Libevent benchmark CLI11)

foreach (TEST_BASE_LIBS ${ALL_PINECONE_TEST_BASE_LIBS})
    find_package(${TEST_BASE_LIBS} REQUIRED)
endforeach ()

set(ALL_PINECONE_TEST_LIBS pinecone rt dl pthread openssl::openssl fmt::fmt boost::boost nlohmann_json::nlohmann_json grpc::grpc glog::glog magic_enum::magic_enum fmt::fmt CLI11::CLI11 libevent::libevent benchmark::benchmark_main)

include_directories("${CMAKE_SOURCE_DIR}/third_party/muduo/")

foreach (TEST_EXECUTABLE ${ALL_PINECONE_TEST_FILES})
    get_filename_component(FILENAME_WITH_EXT ${TEST_EXECUTABLE} NAME)
    string(REGEX REPLACE "\\.[^.]*$" "" FILENAME_WITHOUT_EXT ${FILENAME_WITH_EXT})
    add_executable(test_3drpart_fmt_${FILENAME_WITHOUT_EXT} ${TEST_EXECUTABLE})
    target_link_libraries(test_3drpart_fmt_${FILENAME_WITHOUT_EXT} ${ALL_PINECONE_TEST_LIBS})
endforeach ()