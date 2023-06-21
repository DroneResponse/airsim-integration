#include <unordered_map>

#include <gazebo/msgs/msgs.hh>
#include <gazebo/transport/transport.hh>

#include "udp_sender.hpp"

#ifndef SEND_DRONE_POSE
#define SEND_DRONE_POSE


class GenerateCbLocalPose {
    public:
        /**
         * constructor
         * @param udpSender a UDPSender object
        */
        GenerateCbLocalPose(UDPSender* udpSender);
        ~GenerateCbLocalPose();
        /**
         * local pose callback where gazebo drone represents airsim drone's global pose
         * @param msg gazebo message
        */
        void cbLocalPose(ConstPosesStampedPtr& msg);
        /**
         * subscribe gazebo node pointer to "~pose/local/info" with cbLocalPose callback
         * @param gazeboNodePtr
        */
        gazebo::transport::SubscriberPtr subscribeGazeboNode(
            gazebo::transport::NodePtr gazeboNodePtr
        );
    private:
        UDPSender* udpSender;
        std::unordered_map<std::string, uint16_t> droneIds;
        uint16_t uniqueDroneCount = 0;
        /**
         * gives a single unique uint16_t id to each unique drone name provided
         * @param droneName unique name of a drone
        */
        void trackDroneIds(std::string droneName);
};

#endif