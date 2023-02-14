#include <sys/socket.h>
#include <sys/types.h>
#include <mutex>
#include <netdb.h>
#include <iostream>
#include <string>
#include "pose.hpp"

#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H


class UDPReceiver {       // The class
    public:             // Access specifier
        UDPReceiver(unsigned short int listener_port);  // Constructor
        /**
         * listens for udp messages on specified receiver port
         * 
         * should be started in it's own thread to constantly listen
         * @param pose_message pose message that should be updated with pose data by udp messages
        */
        void listen_pose_message(
            PoseTransfer::PoseMessage &pose_message,
            std::mutex mutex_pose_message);
    private:
        unsigned short int listener_port;
        int sock;
        /**
         * translates uint64_t based pose message to double based pose message
         * @param udp_pose_message network byte order unint64_t udp pose message
         * @param pose_message will be filled with data from udp_pose_message in host byte order
        */
        void udp_message_to_pose(
            PoseTransfer::UdpPoseMessage udp_pose_message,
            PoseTransfer::PoseMessage &pose_message
        );
        double udp_uint64_to_double(uint64_t u_field);
};

#endif