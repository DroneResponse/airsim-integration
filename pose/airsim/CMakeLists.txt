cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(receive_drone_pose)

set (CMAKE_CXX_STANDARD 20)

add_subdirectory(../pose_send_receive ./pose_send_receive)
add_subdirectory(receive_drone_pose)
