# gstreamer pkg-config setup instructions: https://gstreamer.freedesktop.org/documentation/installing/on-mac-osx.html#manual-compilation-with-pkgconfig
# need to set PKG_CONFIG_PATH and prepend gstreamer specific pkg-config to PATH
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(AirSimCamSend)

set(AIRSIM_ROOT /Users/jasonbrauer/drone_response/AirSim)
set(GSTREAMER_ROOT /Library/Frameworks/GStreamer.framework)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

# find_package(gazebo REQUIRED)
find_package(PkgConfig REQUIRED)

# list pkg config libraries with 'pkg-config --list-all'
pkg_check_modules(GST REQUIRED gstreamer-1.0
                    gstreamer-plugins-base-1.0
                    gstreamer-base-1.0
                    gstreamer-app-1.0)

include_directories(
    # ${GAZEBO_INCLUDE_DIRS}
    ${AIRSIM_ROOT}/AirLib/deps/eigen3
    ${AIRSIM_ROOT}/AirLib/deps/rpclib/include
    ${AIRSIM_ROOT}/AirLib/include
    ${GST_INCLUDE_DIRS}
)

link_directories(
    # ${GAZEBO_LIBRARY_DIRS}
    ${AIRSIM_ROOT}/build_release/output/lib
    ${GST_LIBRARY_DIRS}
)

# list(APPEND CMAKE_CXX_FLAGS "${GAZEBO_CXX_FLAGS}")
list(APPEND CMAKE_CXX_FLAGS)

add_executable(${PROJECT_NAME} src/airsim-cam-send.cpp)

target_link_libraries(
    ${PROJECT_NAME} 
    # ${GAZEBO_LIBRARIES}
    pthread
    AirLib
    rpc
    ${GST_LIBRARIES}
)

