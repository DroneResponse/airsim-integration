cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

find_package(gazebo REQUIRED)

link_directories(
    ${GAZEBO_LIBRARY_DIRS}
)

add_executable(send_drone_pose ./send_drone_pose.cpp)

target_link_libraries(
    send_drone_pose
    ${GAZEBO_LIBRARIES}
    pthread
    gazebo_drone_pose
    pose_send_udp
)