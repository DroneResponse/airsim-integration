#include <mutex>

#include "vehicle_pose.hpp"
#include "pose.hpp"


namespace PoseHandlers {
    /**
     * sets drone pose in airsim to gazebo drone pose
     * @param vehicle_interface pointer to vehicle interface
     * @param pose_message reference to a pose message
     * @param mutex_pose_message mutex to lock access to provided pose message when reading
     * @param exit_flag true (1) continues execution while false (0) exits
    */
    void set_drone_pose (
        SimulatorInterface::VehiclePose *vehicle_interface,
        PoseTransfer::PoseMessage *pose_message,
        std::mutex *mutex_pose_message,
        bool *exit_flag
    );

}