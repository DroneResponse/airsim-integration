cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(test_gazebo_drone_pose)

set (CMAKE_CXX_STANDARD 20)

find_package(gazebo REQUIRED)

# include googletest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG 001f281926c74a819e9266bb4ec525515ff39418 # latest verified commit on main 06/13/2023
)
FetchContent_MakeAvailable(googletest)

# mocks for send / receive interfaces
include_directories(
    ../../test/pose_send_receive
)

link_directories(
    ${GAZEBO_LIBRARY_DIRS}
)

# add any additional test files as sources
add_executable(
    test_all
    test_gazebo_drone_pose.cpp
)

target_link_libraries(
    test_all
    ${GAZEBO_LIBRARIES}
    gazebo_drone_pose
    pose_send_udp
    GTest::gtest_main
    GTest::gtest
    GTest::gmock
)