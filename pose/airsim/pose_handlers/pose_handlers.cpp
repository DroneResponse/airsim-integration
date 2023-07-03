#include <thread>

#include "pose_handlers.hpp"


void PoseHandlers::set_drone_pose(
    SimulatorInterface::VehiclePose &vehicle_interface,
    PoseTransfer::PoseMessage &pose_message,
    std::mutex &mutex_pose_message
) {
    uint64_t msg_count = 0;
    while(1) {
        mutex_pose_message.lock();
        if (pose_message.message_counter > msg_count) {
            
            vehicle_interface.set_vehicle_pose(pose_message.drone, "");

        }
        msg_count = pose_message.message_counter;
        mutex_pose_message.unlock();
        // update at ~200hz
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}