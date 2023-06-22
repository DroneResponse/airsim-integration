#include <string>

#include "pose.hpp"


#ifndef POSE_SENDER_H
#define POSE_SENDER_H

/** Abstract class for sending pose messages over a network
*/
class PoseSender {       // The class
    public:             // Access specifier
        PoseSender(std::string host, unsigned short int dest_port) {};  // Constructor
        virtual ~PoseSender() {};
        /** Opens socket for initialized host and port
        */
        virtual void create_socket() {};
        /** Sends pose message to destination host and port
        * @param pose_message message to be sent over udp
        */
        virtual void send_pose_message(const PoseTransfer::PoseMessage pose_message) {};  // Method/function declaration
    private:
        unsigned short int dest_port;
        std::string host;
        int sock;
};


#endif