#ifndef POSE_H
#define POSE_H

// we need a struct that has fields for
// drone position and attitude
// camera position and attitude
#pragma pack
typedef struct {
    double x;
    double y;
    double z;
    double w;
    double xi;
    double yj;
    double zk;
} Pose;

#pragma pack
typedef struct {
    Pose drone;
    Pose camera;
} PoseMessage;

#endif