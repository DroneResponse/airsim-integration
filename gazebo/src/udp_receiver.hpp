#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include "pose.hpp"

#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H


class UDPReceiver {       // The class
    public:             // Access specifier
        UDPReceiver(unsigned short int listener_port);  // Constructor
        PoseTransfer::PoseMessage listen_pose_message();  // Method/function declaration
    private:
        unsigned short int listener_port;
        int sock;
        PoseTransfer::PoseMessage udp_message_to_pose(PoseTransfer::UdpPoseMessage);
};

#endif