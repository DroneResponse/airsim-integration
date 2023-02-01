cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(AirSimCamSend)

if (NOT DEFINED ENV{AIRSIM_ROOT})
    message(FATAL_ERROR "set the env variable AIRSIM_ROOT to the AirSim install directory")
else()
    message(STATUS "AIRSIM_ROOT=" $ENV{AIRSIM_ROOT})
endif()

if (NOT DEFINED ENV{GSTREAMER_ROOT})
    message(FATAL_ERROR "set the env variable GSTREAMER_ROOT to to Gstreamer.framework directory")
else()
    message(STATUS "GSTREAMER_ROOT=" $ENV{GSTREAMER_ROOT})
endif()

if (NOT DEFINED ENV{PKG_CONFIG_PATH})
    message(WARNING "if compiling on Mac, gstreamer docs expect the PKG_CONFIG_PATH env variable to be set: https://gstreamer.freedesktop.org/documentation/installing/on-mac-osx.html#manual-compilation-with-pkgconfig")
else()
    message(STATUS "PKG_CONFIG_PATH=" $ENV{PKG_CONFIG_PATH})
endif()

set(AIRSIM_ROOT $ENV{AIRSIM_ROOT})
set(GSTREAMER_ROOT $ENV{GSTREAMER_ROOT})

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

find_package(PkgConfig REQUIRED)

# list pkg config libraries with 'pkg-config --list-all'
pkg_check_modules(GST REQUIRED gstreamer-1.0
                    gstreamer-plugins-base-1.0
                    gstreamer-base-1.0
                    gstreamer-app-1.0)

# ideally use ${GST_INCLUDE_DIRS}, but not successfully finding /Headers
include_directories(
    ${AIRSIM_ROOT}/AirLib/deps/eigen3
    ${AIRSIM_ROOT}/AirLib/deps/rpclib/include
    ${AIRSIM_ROOT}/AirLib/include
    ${GSTREAMER_ROOT}/Headers
)

link_directories(
    ${AIRSIM_ROOT}/build_release/output/lib
    ${GSTREAMER_ROOT}/Libraries
)

list(APPEND CMAKE_CXX_FLAGS)

add_executable(${PROJECT_NAME} src/airsim-cam-send.cpp)

target_link_libraries(
    ${PROJECT_NAME} 
    pthread
    AirLib
    rpc
    ${GST_LIBRARIES}
)

