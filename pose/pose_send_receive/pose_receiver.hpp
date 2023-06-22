#include <mutex>

#include "pose.hpp"

#ifndef POSE_RECEIVER_H
#define POSE_RECEIVER_H


class PoseReceiver {       // The class
    public:             // Access specifier
        PoseReceiver(unsigned short int listener_port) {};  // Constructor
        virtual ~PoseReceiver() {};
        /**
         * listens for udp messages on specified receiver port
         * 
         * should be started in it's own thread to constantly listen
         * @param pose_message pose message that should be updated with pose data by udp messages
         * @param mutex_pose_message outside mutex reference to lock when updating pose_message
        */
        virtual void listen_pose_message(
            PoseTransfer::PoseMessage *pose_message,
            std::mutex &mutex_pose_message
        ) {};
    private:
        unsigned short int listener_port;
        int sock;
};


#endif