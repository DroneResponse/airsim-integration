#include <gazebo/msgs/msgs.hh>

#include "udp_sender.hpp"

#ifndef SEND_DRONE_POSE_PI
#define SEND_DRONE_POSE_PI

/* Export cbLocalPose for testing
*/
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
    private:
        UDPSender& udpSender;
};

#endif