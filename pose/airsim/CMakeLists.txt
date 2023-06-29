cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(receive_drone_pose)

if (NOT DEFINED ENV{AIRSIM_ROOT})
    message(FATAL_ERROR "set the env variable AIRSIM_ROOT to the AirSim install directory")
else()
    message(STATUS "AIRSIM_ROOT=" $ENV{AIRSIM_ROOT})
endif()

set(AIRSIM_ROOT $ENV{AIRSIM_ROOT})

set (CMAKE_CXX_STANDARD 20)

set(UdpSourceDir
    ../src
)
set(UdpSourceFiles
    ${UdpSourceDir}/udp_receiver.cpp
)

include_directories(
    ${UdpSourceDir}
    ${AIRSIM_ROOT}/AirLib/deps/eigen3
    ${AIRSIM_ROOT}/AirLib/deps/rpclib/include
    ${AIRSIM_ROOT}/AirLib/include
)

link_directories(
    ${AIRSIM_ROOT}/build_release/output/lib
)

if(WIN32)
    include_directories(
        "${AIRSIM_ROOT}/AirLib/include"

    )
    link_directories(
        "${AIRSIM_ROOT}/AirLib/lib/x64/Release"
    )
    
endif()

add_executable(${PROJECT_NAME} src/receive_drone_pose.cpp ${UdpSourceFiles})

if (WIN32)
    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE ${AIRSIM_ROOT}/AirLib/lib/x64/Release/AirLib.lib
        PRIVATE ${AIRSIM_ROOT}/AirLib/deps/rpclib/lib/x64/Release/rpc.lib
        PRIVATE ${AIRSIM_ROOT}/AirLib/deps/MavLinkCom/lib/x64/Release/MavLinkCom.lib
    )
else()
    target_link_libraries(
        ${PROJECT_NAME}
        rpc
        AirLib
    )
endif()