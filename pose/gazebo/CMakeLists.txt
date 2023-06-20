cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(send_drone_pose)

set (CMAKE_CXX_STANDARD 20)

# out of tree dir, so need to place generated binary libs in build
# https://stackoverflow.com/questions/35260552/how-do-i-explicitly-specify-an-out-of-tree-source-in-cmake
add_subdirectory(../pose_send_receive ./pose_send_receive)
add_subdirectory(gazebo_drone_pose)

find_package(gazebo REQUIRED)

include_directories(
    ${GAZEBO_INCLUDE_DIRS}
)

link_directories(
    ${GAZEBO_LIBRARY_DIRS}
)

list(APPEND CMAKE_CXX_FLAGS "${GAZEBO_CXX_FLAGS}")

add_executable(${PROJECT_NAME} ./send_drone_pose/send_drone_pose.cpp)

target_link_libraries(
    ${PROJECT_NAME}
    ${GAZEBO_LIBRARIES}
    pthread
    gazebo_drone_pose
    pose_send_udp
)