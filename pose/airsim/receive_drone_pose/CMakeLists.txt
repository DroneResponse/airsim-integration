cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if (NOT DEFINED ENV{AIRSIM_ROOT})
    message(FATAL_ERROR "set the env variable AIRSIM_ROOT to the AirSim install directory")
else()
    message(STATUS "AIRSIM_ROOT=" $ENV{AIRSIM_ROOT})
endif()

set(AIRSIM_ROOT $ENV{AIRSIM_ROOT})

include_directories(
    ${AIRSIM_ROOT}/AirLib/deps/eigen3
    ${AIRSIM_ROOT}/AirLib/deps/rpclib/include
    ${AIRSIM_ROOT}/AirLib/include
)

link_directories(
    ${AIRSIM_ROOT}/build_release/output/lib
)

add_executable(receive_drone_pose receive_drone_pose.cpp pose_handlers.cpp)

target_link_libraries(
    receive_drone_pose
    rpc
    AirLib
    airsim_pose
    pose_receive_udp
)
    
