cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(receive_drone_pose)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

set(UdpSourceDir
    ../src
)
set(UdpSourceFiles
    ${UdpSourceDir}/udp_receiver.cpp
)

include_directories(
    ${UdpSourceDir}
)

add_executable(${PROJECT_NAME} src/receive_drone_pose.cpp ${UdpSourceFiles})