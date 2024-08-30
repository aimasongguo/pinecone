string (TIMESTAMP BUILD_TIME)

find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --abbrev=6 --dirty --always --tags
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE  GIT_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=short
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE  GIT_DATE
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%H
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE  GIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()
