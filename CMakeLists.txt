cmake_minimum_required(VERSION 3.8)

project(rkpartybox)

add_definitions(-g -D__EXPORTED_HEADERS__)

include_directories(
	${PROJECT_SOURCE_DIR}/sys/
	${PROJECT_SOURCE_DIR}/cJSON/
	${PROJECT_SOURCE_DIR}/include/
	${CMAKE_SYSROOT}/usr/include/rockchip/
	)

include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${DBUS_INCLUDE_DIRS})

aux_source_directory(${PROJECT_SOURCE_DIR}/sys SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/cJSON SRCS)
aux_source_directory(./app SRCS)
aux_source_directory(./sys SRCS)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
message("CMAKE_C_FLAGS_DEBUG: ${CMAKE_C_FLAGS_DEBUG}")

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME}
	pthread m freetype rkwifibt
)

set(ROCKIT_FILE_LIBS
       ./lib64/librockit.so
       ./lib64/libaec_bf_process.so
       ./lib64/libavutil.so.56
       ./lib64/librkaudio_effect_eqdrc.so
       ./lib64/libavcodec.so.58      
       ./lib64/libdrm.so.2           
       ./lib64/librkaudio_effect_reverb.so  
       ./lib64/libswresample.so.3
       ./lib64/libavformat.so.58     
       ./lib64/librkaudio_common.so  
       ./lib64/librkaudio_effect_vocal.so
       ./lib64/librkwifibt.so
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${ROCKIT_FILE_LIBS} DESTINATION "lib")
install(FILES bins/asound.conf DESTINATION /etc)
install(PROGRAMS bins/S99-rkpartybox DESTINATION /etc/init.d)
