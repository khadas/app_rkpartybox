cmake_minimum_required(VERSION 3.8)

project(rkpartybox)

add_definitions(-g -D__EXPORTED_HEADERS__)

include_directories(
	${PROJECT_SOURCE_DIR}/app/
	${PROJECT_SOURCE_DIR}/modules/uac/
	${PROJECT_SOURCE_DIR}/modules/userial/
	${PROJECT_SOURCE_DIR}/modules/storage/
	${PROJECT_SOURCE_DIR}/sys/
	${PROJECT_SOURCE_DIR}/utils/log/
	${PROJECT_SOURCE_DIR}/utils/os/
	${PROJECT_SOURCE_DIR}/cJSON/
	${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/hal/
	${CMAKE_SYSROOT}/usr/include/rockchip/
	)

include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${DBUS_INCLUDE_DIRS})

aux_source_directory(${PROJECT_SOURCE_DIR}/sys SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/cJSON SRCS)
aux_source_directory(./app SRCS)
aux_source_directory(./modules/uac SRCS)
aux_source_directory(./modules/userial SRCS)
aux_source_directory(./modules/storage SRCS)
aux_source_directory(./sys SRCS)
aux_source_directory(./utils/log SRCS)
aux_source_directory(./utils/os SRCS)
aux_source_directory(./hal SRCS)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O2")
message("CMAKE_C_FLAGS_DEBUG: ${CMAKE_C_FLAGS_DEBUG}")
message("CMAKE_SYSROOT: ${CMAKE_SYSROOT}")
if (LV_USE_DEMO_MUSIC)
add_definitions(-DLV_USE_DEMO_MUSIC=1)
include_directories(${PROJECT_SOURCE_DIR}/app/music_ui/)
include_directories(${PROJECT_SOURCE_DIR}/app/music_ui/lvgl)
include_directories(${CMAKE_SYSROOT}/usr/include/libdrm/)
include_directories(${CMAKE_SYSROOT}/usr/include/lvgl/)
include_directories(${CMAKE_SYSROOT}/usr/include/lvgl/lv_drivers/)
aux_source_directory(${PROJECT_SOURCE_DIR}/lvgl/ SRCS)
aux_source_directory(./app/music_ui SRCS)
aux_source_directory(./app/music_ui/lvgl SRCS)
aux_source_directory(./app/music_ui/assets SRCS)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(UDEV libudev REQUIRED)
include_directories(${UDEV_INCLUDE_DIRS})

link_directories(${PROJECT_SOURCE_DIR}/lib64/bt)
add_executable(${PROJECT_NAME} ${SRCS})

target_compile_definitions(${PROJECT_NAME} PRIVATE -D_GNU_SOURCE)
target_link_libraries(${PROJECT_NAME} ${UDEV_LIBRARIES})
target_link_libraries(${PROJECT_NAME}
	pthread m freetype rkwifibt
)

if (LV_USE_DEMO_MUSIC)
target_link_libraries(${PROJECT_NAME} lvgl lv_drivers drm)
endif()

if(RK3308_PBOX_CORE_BOARD)
	add_definitions(-DENABLE_EXT_BT_MCU)
	install(FILES ./lib64/librkaudio_effect_howling.so DESTINATION "lib" RENAME "librkaudio_effect_howling.so")
	install(FILES ./lib64/librockit.so DESTINATION "lib" RENAME "librockit.so")
	install(FILES ./bins/eq_drc_recorder_vendor_2chn.bin DESTINATION /etc/pbox RENAME "eq_drc_recorder.bin")
	install(FILES ./bins/rkstudio_vendor_2in6out.bin DESTINATION /etc/pbox RENAME "rkstudio.bin")
	install(PROGRAMS bins/partybox_app_vendor.sh DESTINATION bin RENAME "partybox_app.sh")
else()
	install(FILES ./lib64/librkaudio_effect_howling.so DESTINATION "lib" RENAME "librkaudio_effect_howling.so")
	install(FILES ./lib64/librockit.so DESTINATION "lib" RENAME "librockit.so")
	install(FILES ./bins/eq_drc_recorder_evb_1chn.bin DESTINATION /etc/pbox RENAME "eq_drc_recorder.bin")
	install(FILES ./bins/rkstudio_evb_2in2out.bin DESTINATION /etc/pbox RENAME "rkstudio.bin")
	install(PROGRAMS bins/partybox_app_evb.sh DESTINATION bin RENAME "partybox_app.sh")
endif()

set(ROCKIT_FILE_LIBS
       ./lib64/libaec_bf_process.so
       ./lib64/libavutil.so.56
       ./lib64/librkaudio_effect_eqdrc.so
       ./lib64/libavcodec.so.58
       ./lib64/libswresample.so.3
       ./lib64/libavformat.so.58
       ./lib64/librkaudio_common.so
       ./lib64/librkaudio_effect_vocal.so
       ./lib64/librkstudio_tuning.so
       ./lib64/librkstudio.so
       ./lib64/libgmp.so.10
       ./lib64/bt/librkwifibt.so
       )

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${ROCKIT_FILE_LIBS} DESTINATION "lib")
install(FILES bins/asound.conf DESTINATION /etc)
install(FILES bins/led_effect.json DESTINATION /etc/pbox)
install(FILES bins/uac_config DESTINATION /etc/pbox)
install(FILES bins/SmileySans-Oblique.ttf DESTINATION /etc/pbox)

install(FILES bins/wozai-48k2ch.pcm DESTINATION /etc/pbox)
install(FILES bins/config_howling.json DESTINATION /etc/pbox)
install(FILES bins/eq_drc_player.bin DESTINATION /etc/pbox)
#install(FILES ./bins/eq_drc_recorder_vendor_2chn.bin DESTINATION /etc/pbox RENAME "eq_drc_recorder.bin")


install(FILES bins/Mono.pcm DESTINATION /etc/pbox)
install(FILES bins/Stereo.pcm DESTINATION /etc/pbox)
install(FILES bins/Widen.pcm DESTINATION /etc/pbox)
install(FILES bins/Split_off.pcm DESTINATION /etc/pbox)
install(FILES bins/Split_on.pcm DESTINATION /etc/pbox)
install(FILES bins/pbox-factory.ini DESTINATION /etc/pbox)
install(PROGRAMS bins/S40-rkpartybox DESTINATION /etc/init.d)
install(PROGRAMS bins/partybox_play.sh DESTINATION bin)
install(PROGRAMS bins/inputkey DESTINATION bin)
