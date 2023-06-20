cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

find_package(gazebo REQUIRED)

set(SendReceiveSourceDir
    ../../pose_send_receive
)

include_directories(
    ${GAZEBO_INCLUDE_DIRS}
    ${SendReceiveSourceDir}
)

add_library(gazebo_drone_pose STATIC gazebo_drone_pose.cpp)

target_include_directories (gazebo_drone_pose PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})