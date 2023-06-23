cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

set (CMAKE_CXX_STANDARD 20)

# include googletest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG 001f281926c74a819e9266bb4ec525515ff39418 # latest verified commit on main 06/13/2023
)
FetchContent_MakeAvailable(googletest)

# add any additional test files as sources
add_executable(
    test_all
    test_receive_drone_pose.cpp
)

target_link_libraries(
    test_all
    receive_drone_pose
    pose_send_udp
    GTest::gtest_main
    GTest::gtest
    GTest::gmock
)