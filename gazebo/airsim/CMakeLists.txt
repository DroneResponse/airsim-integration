cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(receive_drone_pose)

if (NOT DEFINED ENV{AIRSIM_ROOT})
    message(FATAL_ERROR "set the env variable AIRSIM_ROOT to the AirSim install directory")
else()
    message(STATUS "AIRSIM_ROOT=" $ENV{AIRSIM_ROOT})
endif()

set(AIRSIM_ROOT $ENV{AIRSIM_ROOT})

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

set(UdpSourceDir
    ../src
)
set(UdpSourceFiles
    ${UdpSourceDir}/udp_receiver.cpp
)

include_directories(
    ${UdpSourceDir}
    ${AIRSIM_ROOT}/AirLib/deps/eigen3
    ${AIRSIM_ROOT}/AirLib/deps/rpclib/include
    ${AIRSIM_ROOT}/AirLib/include
)

link_directories(
    ${AIRSIM_ROOT}/build_release/output/lib
)

add_executable(${PROJECT_NAME} src/receive_drone_pose.cpp ${UdpSourceFiles})

target_link_libraries(
    ${PROJECT_NAME}
    rpc
    AirLib
)