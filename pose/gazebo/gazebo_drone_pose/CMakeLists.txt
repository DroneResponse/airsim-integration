cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

find_package(gazebo REQUIRED)

set(SendReceiveSourceDir
../../pose_send_receive
)

# ideally would reference target pose_send_udp built with add_subdirectory
# in top level CMakeLists, but haven't found a way to do so
include_directories(
    ${SendReceiveSourceDir}   
)

# creates a target as library called 'gazebo_drone_pose'
add_library(gazebo_drone_pose STATIC gazebo_drone_pose.cpp)
    
target_include_directories (
    gazebo_drone_pose PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${GAZEBO_INCLUDE_DIRS}
)