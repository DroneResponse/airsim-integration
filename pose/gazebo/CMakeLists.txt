cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(send_drone_pose)

set (CMAKE_CXX_STANDARD 20)

# include googletest
# latest verified commit on main 06/13/2023
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/001f281926c74a819e9266bb4ec525515ff39418.zip
)
FetchContent_MakeAvailable(googletest)

set(UdpSourceDir
    ../src
)
set(UdpSourceFiles
    ${UdpSourceDir}/udp_sender.cpp
)
set(DronePoseSourceDir
    ../src
)
set(DronePoseSourceFiles
    ${DronePoseSourceDir}/gazebo_drone_pose.cpp
)

find_package(gazebo REQUIRED)

include_directories(
    ${GAZEBO_INCLUDE_DIRS}
    ${UdpSourceDir}
    ${DronePoseSourceDir}
)

link_directories(
    ${GAZEBO_LIBRARY_DIRS}
)

list(APPEND CMAKE_CXX_FLAGS "${GAZEBO_CXX_FLAGS}")

add_executable(${PROJECT_NAME} src/send_drone_pose.cpp ${DronePoseSourceFiles} ${UdpSourceFiles} )

target_link_libraries(
    ${PROJECT_NAME}
    ${GAZEBO_LIBRARIES}
    pthread
)