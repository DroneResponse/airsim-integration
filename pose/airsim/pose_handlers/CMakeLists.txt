cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

add_library(pose_handlers STATIC pose_handlers.cpp)
target_include_directories(
    pose_handlers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../vehicle_pose
)

target_link_libraries(
    pose_handlers
    pose_receive_udp
)
    
