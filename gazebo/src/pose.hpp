#ifndef POSE_H
#define POSE_H

#include <stdint.h>

namespace PoseTransfer {
    // we need a struct that has fields for
    // drone position and attitude
    // camera position and attitude
    typedef struct Pose {
        double x;
        double y;
        double z;
        double w;
        double xi;
        double yj;
        double zk;
    } Pose;

    typedef struct PoseMessage {
        uint64_t message_counter; // this is a counter that increments every time we send a message
        Pose drone;
        Pose camera;
    } PoseMessage;

    #pragma pack(push,1)
    typedef struct UdpPose {
        uint64_t x;
        uint64_t y;
        uint64_t z;
        uint64_t w;
        uint64_t xi;
        uint64_t yj;
        uint64_t zk;
    } UdpPose;

    typedef struct UdpPoseMessage {
        uint64_t message_counter; // this is a counter that increments every time we send a message
        UdpPose drone;
        UdpPose camera;
    } UdpPoseMessage;
    #pragma pack(pop)

    static const unsigned long udp_decimal_offset = 1e5; // used to convert doubles to uint64_t
}


#endif