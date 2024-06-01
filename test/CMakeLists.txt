cmake_minimum_required(VERSION 3.8)

set(INPUTKEY_FILES
    inputkey.c
)

add_definitions("")
add_executable(inputkey ${INPUTKEY_FILES})
install(TARGETS inputkey DESTINATION bin)