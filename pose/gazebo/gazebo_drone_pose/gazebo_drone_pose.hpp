#include <unordered_map>
#include <string>

#include <gazebo/msgs/msgs.hh>
#include <gazebo/transport/transport.hh>

#include "pose_sender.hpp"
#include "pose.hpp"

#ifndef SEND_DRONE_POSE
#define SEND_DRONE_POSE


class GenerateCbLocalPose {
    public:
        /**
         * constructor
         * @param poseSender a UDPSender object
        */
        GenerateCbLocalPose(PoseSender* poseSender, std::string drone_name);
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

        std::string getCurrentTimeInFormat();

    private:
        PoseSender* poseSender;
        std::string drone_name;
        std::unordered_map<std::string, uint16_t> droneIds;
        uint16_t uniqueDroneCount = 0;
        /**
         * gives a single unique uint16_t id to each unique drone name provided
         * @param droneName unique name of a drone
        */
        void trackDroneIds(const std::string droneName);
        void resetPoseToDefault(uint16_t key);

        std::unordered_map<uint16_t, PoseTransfer::Pose> drone_pose_map;
        std::unordered_map<uint16_t, PoseTransfer::Pose> camera_pose_map;

};

#endif
