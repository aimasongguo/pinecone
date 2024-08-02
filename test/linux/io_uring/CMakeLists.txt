include_directories(${CMAKE_SOURCE_DIR})

file(GLOB_RECURSE ALL_PINECONE_TEST_FILES "./*.c")

foreach (TEST_EXECUTABLE ${ALL_PINECONE_TEST_FILES})
    get_filename_component(FILENAME_WITH_EXT ${TEST_EXECUTABLE} NAME)
    string(REGEX REPLACE "\\.[^.]*$" "" FILENAME_WITHOUT_EXT ${FILENAME_WITH_EXT})
    add_executable(test_linux_io_uring_${FILENAME_WITHOUT_EXT} ${TEST_EXECUTABLE})
endforeach ()