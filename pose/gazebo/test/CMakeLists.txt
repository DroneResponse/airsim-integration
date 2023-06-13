cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(test_gazebo_drone_pose)

set (CMAKE_CXX_STANDARD 20)

# include googletest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG 001f281926c74a819e9266bb4ec525515ff39418 # latest verified commit on main 06/13/2023
)
FetchContent_MakeAvailable(googletest)

set(DronePoseSourceDir
    ../src
)
set(DronePoseSourceFiles
    ${DronePoseSourceDir}/gazebo_drone_pose.cpp
)
set(UdpSourceDir
    ../../src
)
set(UdpSourceFiles
    ${UdpSourceDir}/udp_sender.cpp
)

# test specific
set(UdpSourceDirTest
    ../../test
)

find_package(gazebo REQUIRED)

include_directories(
    ${DronePoseSourceDir}
    ${GAZEBO_INCLUDE_DIRS}
    ${UdpSourceDir}
    ${UdpSourceDirTest}
)

link_directories(
    ${GAZEBO_LIBRARY_DIRS}
)

list(APPEND CMAKE_CXX_FLAGS "${GAZEBO_CXX_FLAGS}")

add_executable(
    ${PROJECT_NAME}
    ./test_gazebo_drone_pose.cpp
    ${DronePoseSourceFiles}
    ${UdpSourceFiles}
    ${UdpSourceFilesTest}
)

target_link_libraries(
    ${PROJECT_NAME}
    ${GAZEBO_LIBRARIES}
    pthread
    GTest::gtest
    GTest::gmock
)