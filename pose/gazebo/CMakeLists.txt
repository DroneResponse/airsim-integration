cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(send_drone_pose)

set (CMAKE_CXX_STANDARD 20)

set(UdpSourceDir
    ../src
)
set(UdpSourceFiles
    ${UdpSourceDir}/udp_sender.cpp
)


find_package(gazebo REQUIRED)

include_directories(
    ${GAZEBO_INCLUDE_DIRS}
    ${UdpSourceDir}
)

link_directories(
    ${GAZEBO_LIBRARY_DIRS}
)

list(APPEND CMAKE_CXX_FLAGS "${GAZEBO_CXX_FLAGS}")

add_executable(${PROJECT_NAME} src/send_drone_pose.cpp ${UdpSourceFiles})

target_link_libraries(
    ${PROJECT_NAME}
    ${GAZEBO_LIBRARIES}
    pthread
)