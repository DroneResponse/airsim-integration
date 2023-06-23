cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if (NOT DEFINED ENV{AIRSIM_ROOT})
    message(FATAL_ERROR "set the env variable AIRSIM_ROOT to the AirSim install directory")
else()
    message(STATUS "AIRSIM_ROOT=" $ENV{AIRSIM_ROOT})
endif()

set(AIRSIM_ROOT $ENV{AIRSIM_ROOT})

link_directories(
    ${AIRSIM_ROOT}/build_release/output/lib
)

add_library(airsim_pose STATIC airsim_pose.cpp)
target_include_directories(
    airsim_pose PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${AIRSIM_ROOT}/AirLib/deps/eigen3
    ${AIRSIM_ROOT}/AirLib/deps/rpclib/include
    ${AIRSIM_ROOT}/AirLib/include
)

target_link_libraries(
    airsim_pose
    rpc
    AirLib
    pose_receive_udp
)

