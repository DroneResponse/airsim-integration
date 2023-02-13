#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include "pose.hpp"

#ifndef UDP_SENDER_H
#define UDP_SENDER_H


class UDPSender {       // The class
    public:             // Access specifier
        UDPSender(std::string host, unsigned short int dest_port);  // Constructor
        void send_pose_message(const PoseTransfer::PoseMessage pose_message);  // Method/function declaration
    private:
        unsigned short int dest_port;
        std::string host;  // Attribute (string variable)
        int sock;
        PoseTransfer::UdpPoseMessage pose_to_udp_message(PoseTransfer::PoseMessage);
};

#endif