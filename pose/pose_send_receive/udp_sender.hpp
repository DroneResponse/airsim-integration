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
#include <iostream>
#include <string>
#include "pose.hpp"
#include "pose_sender.hpp"

#ifndef UDP_SENDER_H
#define UDP_SENDER_H


class UDPSender : public PoseSender {       // The class
    public:             // Access specifier
        UDPSender (std::string host, unsigned short int dest_port);  // Constructor
        ~UDPSender();
        /** Opens socket for initialized host and port
        */
        void create_socket();
        /** Sends pose message to destination host and port
        * @param pose_message message to be sent over udp
        */
        void send_pose_message(const PoseTransfer::PoseMessage pose_message);  // Method/function declaration
    private:
        unsigned short int dest_port;
        std::string host;
        int sock;
        PoseTransfer::UdpPoseMessage pose_to_udp_message(PoseTransfer::PoseMessage pose_message);
        int64_t double_to_udp_int64(double d_field);
};

#endif