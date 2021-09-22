cmake_minimum_required (VERSION 2.8)

#项目名称
project(demo)
set(PROJECT demo)

#使用c++11标准
add_definitions(-std=c++11)

#配置Debug或Release版本
set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_BUILD_TYPE "Release")

if(DEFINED ${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -rdynamic -O2 -Wall")
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -rdynamic -O2 -Wall")
else()
    set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -rdynamic -O0 -Wall -g2 -ggdb")
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -rdynamic -O0 -Wall -g2 -ggdb")
endif()

#引入头文件
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/app/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/libx264/include
)

#引入源文件
file(GLOB SRC_LIST "app/src/*.cpp")

#生成可执行文件
add_executable(${PROJECT} ${SRC_LIST}) 

#生成动态库
#add_library(${PROJECT} SHARED ${SRC_LIST})

#生成静态库
#add_library(${PROJECT} STATIC ${SRC_LIST})

#链接静态库和动态库
# target_link_libraries(${PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/libs/libzlog/lib/libzlog.a)

target_link_libraries(
    ${PROJECT}
    -Wl,--start-group
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/lib/libavcodec.a
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/lib/libavdevice.a
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/lib/libavfilter.a
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/lib/libavformat.a
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/lib/libavutil.a
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/lib/libpostproc.a
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/lib/libswresample.a
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/lib/libswscale.a
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/libx264/lib/libx264.a
    z
    pthread
    -Wl,--end-group
)

