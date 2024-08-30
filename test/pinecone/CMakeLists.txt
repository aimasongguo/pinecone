include(${CMAKE_SOURCE_DIR}/cmake/pinecone.cmake)

set(UNIT_TEST_EXECUTABLE_NAME run_all_pinecone_unit_test)

add_executable(${UNIT_TEST_EXECUTABLE_NAME})

file(GLOB_RECURSE ALL_PCSG_TEST_SOURCE_FILES
        "unit_test/*.cc"
)

set(PCSG_BASE_LINK_LIBS pinecone gtest gtest_main)

target_sources(${UNIT_TEST_EXECUTABLE_NAME} PRIVATE ${ALL_PCSG_TEST_SOURCE_FILES})
target_include_directories(${UNIT_TEST_EXECUTABLE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}")
target_link_libraries(${UNIT_TEST_EXECUTABLE_NAME} PRIVATE ${PCSG_BASE_LINK_LIBS})

target_compile_definitions(${UNIT_TEST_EXECUTABLE_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_compile_definitions(${UNIT_TEST_EXECUTABLE_NAME} PRIVATE GIT_VERSION="${GIT_VERSION}")
target_compile_definitions(${UNIT_TEST_EXECUTABLE_NAME} PRIVATE GIT_DATE="${GIT_DATE}")
target_compile_definitions(${UNIT_TEST_EXECUTABLE_NAME} PRIVATE GIT_HASH="${GIT_HASH}")
target_compile_definitions(${UNIT_TEST_EXECUTABLE_NAME} PRIVATE BUILD_TIME="${BUILD_TIME}")
