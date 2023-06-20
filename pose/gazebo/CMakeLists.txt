cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(send_drone_pose)

set (CMAKE_CXX_STANDARD 20)

set(SendReceiveSourceDir
    ../pose_send_receive
)
set(SendReceiveSourceFiles
    ${SendReceiveSourceDir}/udp_sender.cpp
)

add_subdirectory(gazebo_drone_pose)

find_package(gazebo REQUIRED)

include_directories(
    ${GAZEBO_INCLUDE_DIRS}
    ${SendReceiveSourceDir}
)

link_directories(
    ${GAZEBO_LIBRARY_DIRS}
)

list(APPEND CMAKE_CXX_FLAGS "${GAZEBO_CXX_FLAGS}")

add_executable(${PROJECT_NAME} ./send_drone_pose/send_drone_pose.cpp ${SendReceiveSourceFiles} )

target_link_libraries(
    ${PROJECT_NAME}
    ${GAZEBO_LIBRARIES}
    pthread
    gazebo_drone_pose
)