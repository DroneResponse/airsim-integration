#include <mutex>

#include "vehicle_pose.hpp"
#include "pose.hpp"


namespace PoseHandlers {
    /**
     * sets drone pose in airsim to gazebo drone pose
     * @param vehicle_interface reference to vehicle interface
     * @param pose_message reference to a pose message
     * @param mutex_pose_message mutex to lock access to provided pose message when reading
    */
    void set_drone_pose (
        SimulatorInterface::VehiclePose &vehicle_interface,
        PoseTransfer::PoseMessage &pose_message,
        std::mutex &mutex_pose_message
    );

}