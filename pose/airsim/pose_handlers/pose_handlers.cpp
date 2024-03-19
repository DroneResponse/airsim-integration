#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "pose_handlers.hpp"

/**
 * spawns a new drone if a new unique identifier is passed
 * @param vehicle_interface pointer to vehicle interface
 * @param drone_id unique vehicle identifier
 * @param initial_pose initial pose when spawned
*/
void spawn_unique_drone(
    SimulatorInterface::VehiclePose *vehicle_interface,
    uint16_t drone_id,
    PoseTransfer::Pose initial_pose
) {
    // because static, will have interactions between tests importing the same translation unit
    static std::vector<uint16_t> unique_drones;
    // if drone_id not found
    // todo: see if can check for existing vehicle_ids != "" and add them to unique drones if exist
    // this should allow the receiver program to restart without having to restart the airsim simulation
    // since can't spawn an already existing drone
    if(std::find(
        unique_drones.begin(),
        unique_drones.end(),
        drone_id) == unique_drones.end()
    ) {
        unique_drones.push_back(drone_id);
        vehicle_interface->spawn_vehicle(std::to_string(drone_id), initial_pose);
        std::cout << "New drone spawned with id: " + std::to_string(drone_id) + "\n" << std::endl;
    }
}


void PoseHandlers::set_drone_pose(
    SimulatorInterface::VehiclePose *vehicle_interface,
    PoseTransfer::PoseMessage *pose_message,
    std::mutex *mutex_pose_message,
    bool *exit_flag
) {
    uint64_t msg_count = 0;
    std::unordered_map<uint16_t, uint64_t> messageCount;

    do {
        msg_count = 0;

        mutex_pose_message->lock();
        // first, have we've ever seen this drone before?
        auto it = messageCount.find(pose_message->drone_id);
        if (it != messageCount.end()) {
            // yes, we have seen this drone before
            // lets record the message count
            msg_count = it->second;
        }
        else {
            // no, we have not seen this drone before
            // let's clear the message count
            // and spawn a new drone
            msg_count = 0;
            spawn_unique_drone(vehicle_interface, pose_message->drone_id, pose_message->drone);
        }

        // Please recall that the message count starts at 1, and increments by 1 each time
        // also each drone has its own message count
        // if the message count is greater than the last message count, then we have new information
        if (pose_message->message_counter > msg_count) {
            // so let's update the drone's pose
            vehicle_interface->set_vehicle_pose(
                pose_message->drone,
                std::to_string(pose_message->drone_id)
            );
            messageCount[pose_message->drone_id] = pose_message->message_counter;;
        }
        mutex_pose_message->unlock();
        // update at ~200hz
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    while(*exit_flag);
}