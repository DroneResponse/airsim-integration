#ifndef POSE_H
#define POSE_H

#include <stdint.h>

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


#endif