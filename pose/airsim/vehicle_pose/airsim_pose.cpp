#include "airsim_pose.hpp"

using namespace SimulatorInterface;


AirSimPose::AirSimPose(void *sim_client) : VehiclePose(sim_client) {
    this->sim_client = sim_client;
}

AirSimPose::spawn_vehicle(std::string vehicle_id) {
    return;
}

AirSimPose::set_vehicle_pose(PoseTransfer::Pose pose, std::string vehicle_id) {
    return;
}