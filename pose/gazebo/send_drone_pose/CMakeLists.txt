cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

find_package(gazebo REQUIRED)

link_directories(
    ${GAZEBO_LIBRARY_DIRS}
)

add_executable(send_drone_pose ./send_drone_pose.cpp)

# gazebo_drone_pose and pose_send_udp targets built and added as libraries at top level CMakeLists
# via add_submodule
target_link_libraries(
    send_drone_pose
    gazebo_drone_pose
    ${GAZEBO_LIBRARIES}
    pthread
    pose_send_udp
)
