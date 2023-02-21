cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(airsim-cam-send)

if (NOT DEFINED ENV{AIRSIM_ROOT})
    message(FATAL_ERROR "set the env variable AIRSIM_ROOT to the AirSim install directory")
else()
    message(STATUS "AIRSIM_ROOT=" $ENV{AIRSIM_ROOT})
    set(AIRSIM_ROOT $ENV{AIRSIM_ROOT})
endif()


if (NOT DEFINED ENV{PKG_CONFIG_PATH})
    message(WARNING "if compiling on Mac, gstreamer docs expect the PKG_CONFIG_PATH env variable to be set: https://gstreamer.freedesktop.org/documentation/installing/on-mac-osx.html#manual-compilation-with-pkgconfig")
else()
    message(STATUS "PKG_CONFIG_PATH=" $ENV{PKG_CONFIG_PATH})
endif()


if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()


if (NOT DEFINED ENV{GSTREAMER_ROOT})
    message(STATUS "Using pkg-config to find gstreamer depenedencies")
else()
    message(STATUS "GSTREAMER_ROOT=" $ENV{GSTREAMER_ROOT})
    message(STATUS "Using the provided GSTREAMER_ROOT env variable to find gstreamer dependencies")
    set(GSTREAMER_ROOT $ENV{GSTREAMER_ROOT})
endif()

find_package(PkgConfig REQUIRED)

# list pkg config libraries with 'pkg-config --list-all'
pkg_check_modules(GST REQUIRED gstreamer-1.0
                    gstreamer-plugins-base-1.0
                    gstreamer-base-1.0
                    gstreamer-app-1.0)

# ideally use ${GST_INCLUDE_DIRS} in all cases,
# but not successfully finding /Headers for mac framework install
if (GSTREAMER_ROOT)
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
else()
include_directories(
    ${AIRSIM_ROOT}/AirLib/deps/eigen3
    ${AIRSIM_ROOT}/AirLib/deps/rpclib/include
    ${AIRSIM_ROOT}/AirLib/include
    ${GST_INCLUDE_DIRS}
)
 message(STATUS "GST_INCLUDE_DIRS=" ${GST_INCLUDE_DIRS})

link_directories(
    ${AIRSIM_ROOT}/build_release/output/lib
    ${GST_LIBRARIES}
    /usr/lib/llvm-8/lib
)
 message(STATUS "GST_LIBRARIES=" ${GST_LIBRARIES})
endif()

list(APPEND CMAKE_CXX_FLAGS)

add_executable(${PROJECT_NAME} src/airsim-cam-send.cpp)

target_link_libraries(
    ${PROJECT_NAME} 
    pthread
    AirLib
    rpc
    ${GST_LIBRARIES}
)

