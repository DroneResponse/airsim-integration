#ifdef _WIN32
    // Windows-specific headers
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>  // For Windows DNS-related functions
#else
    // Unix-specific headers
    #include <sys/socket.h>
    #include <netdb.h>
#endif

#ifndef SHUT_RDWR
    /*
    SHUT_RDWR is defined in sys/socket.h
    but on windows winsock2.h defines SD_BOTH instead.
    */
    #define SHUT_RDWR SD_BOTH 
#endif

#include <sys/types.h>
#include <mutex>
#include <iostream>
#include <string>
#include "pose.hpp"
#include "pose_receiver.hpp"

#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H


class UDPReceiver : public PoseReceiver {       // The class
    public:             // Access specifier
        UDPReceiver(unsigned short int listener_port);  // Constructor
        ~UDPReceiver();
        /**
         * listens for udp messages on specified receiver port
         * 
         * should be started in it's own thread to constantly listen
         * @param pose_message pose message that should be updated with pose data by udp messages
         * @param mutex_pose_message outside mutex reference to lock when updating pose_message
        */
        void listen_pose_message(
            PoseTransfer::PoseMessage *pose_message,
            std::mutex &mutex_pose_message);
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
            PoseTransfer::PoseMessage *pose_message
        );
        double udp_int64_to_double(int64_t u_field);
};

#endif