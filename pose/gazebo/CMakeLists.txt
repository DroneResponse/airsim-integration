# Organized similar to: http://www.kaizou.org/2014/11/typical-cmake-project.html
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(send_drone_pose)

set (CMAKE_CXX_STANDARD 20)

# out of tree dir, so need to place generated binary libs in build
# https://stackoverflow.com/questions/35260552/how-do-i-explicitly-specify-an-out-of-tree-source-in-cmake
add_subdirectory(../pose_send_receive ./pose_send_receive)
add_subdirectory(gazebo_drone_pose)
add_subdirectory(send_drone_pose)
add_subdirectory(test)

list(APPEND CMAKE_CXX_FLAGS "${GAZEBO_CXX_FLAGS}")