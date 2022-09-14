PROJECT(ffmpeg_tutorial)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

add_definitions(-g -O0 -ggdb -gdwarf -funwind-tables -rdynamic)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-D_GLIBCXX_USE_C99=1)

set(CMAKE_C_FLAGS "-Wno-attributes -Wno-write-strings -Wno-deprecated-declarations ${CMAKE_C_FLAGS} -s")
set(CMAKE_CXX_FLAGS "-Wno-attributes -Wno-write-strings -Wno-deprecated-declarations ${CMAKE_CXX_FLAGS} -s")

aux_source_directory(. SRC_LIST)

# target
include_directories(${CMAKE_INSTALL_PREFIX}/include)
link_directories(${CMAKE_INSTALL_PREFIX}/lib)

# opencv
include_directories(${CMAKE_INSTALL_PREFIX}/include/opencv4)

# ffmpeg_tutorial headers
include_directories(${PROJECT_SOURCE_DIR}/rockchip)
link_directories(${PROJECT_SOURCE_DIR}/rockchip)

# rockchip
aux_source_directory(./rockchip RK_SRCS)
list(APPEND SRC_LIST ${RK_SRCS})

add_executable(ffmpeg_tutorial ${SRC_LIST})
target_link_libraries(ffmpeg_tutorial rga drm rknn_api mali)
# target_link_libraries(ffmpeg_tutorial SDL2)
target_link_libraries(ffmpeg_tutorial opencv_imgcodecs opencv_features2d opencv_imgproc opencv_videoio opencv_highgui opencv_core)
target_link_libraries(ffmpeg_tutorial avcodec swresample swscale avfilter avdevice avutil avformat)
target_link_libraries(ffmpeg_tutorial -pthread stdc++ dl)

INSTALL(TARGETS ffmpeg_tutorial DESTINATION bin)
