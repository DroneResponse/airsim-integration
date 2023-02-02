cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(DronePose)

if (NOT DEFINED ENV{AIRSIM_ROOT})
    message(FATAL_ERROR "set the env variable AIRSIM_ROOT to the AirSim install directory")
else()
    message(STATUS "AIRSIM_ROOT=" $ENV{AIRSIM_ROOT})
endif()

set(AIRSIM_ROOT $ENV{AIRSIM_ROOT})

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

find_package(gazebo REQUIRED)

include_directories(
    ${GAZEBO_INCLUDE_DIRS}
    ${AIRSIM_ROOT}/AirLib/deps/eigen3
    ${AIRSIM_ROOT}/AirLib/deps/rpclib/include
    ${AIRSIM_ROOT}/AirLib/include
)

link_directories(
    ${GAZEBO_LIBRARY_DIRS}
    ${AIRSIM_ROOT}/build_release/output/lib
)

list(APPEND CMAKE_CXX_FLAGS "${GAZEBO_CXX_FLAGS}")

add_executable(${PROJECT_NAME} src/drone_pose.cpp)

target_link_libraries(
    ${PROJECT_NAME} 
    ${GAZEBO_LIBRARIES}
    pthread
    AirLib
    rpc
)