cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

add_library(pose_send_udp STATIC udp_sender.cpp)
target_include_directories (pose_send_udp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(pose_receive_udp STATIC udp_receiver.cpp)
target_include_directories (pose_receive_udp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


