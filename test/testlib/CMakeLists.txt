cmake_minimum_required(VERSION 3.8)

set(TESTLIB_FILES
    testlib.c
)

# add lib target file
add_library(mytest STATIC ${TESTLIB_FILES})

# specify the HEAD files
target_include_directories(mytest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# install it if you want
#install(TARGETS mytest ARCHIVE DESTINATION lib)