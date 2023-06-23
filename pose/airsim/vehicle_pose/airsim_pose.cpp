#include "airsim_pose.hpp"

using namespace SimulatorInterface;


AirSimPose::AirSimPose(void *sim_client) : VehiclePose(sim_client) {
    // may not need sim_client if sim specific client type cast works
    this->sim_client = sim_client;
    // type case generic client pointer in AirSim specific client pointer
    this->airsim_client = (msr::airlib::MultirotorRpcLibClient*) sim_client;
};

void AirSimPose::spawn_vehicle(std::string vehicle_id) {
    return;
};

void AirSimPose::set_vehicle_pose(PoseTransfer::Pose pose, std::string vehicle_id) {
    msr::airlib::Vector3r vehicle_position (
        (float) pose.x,
        (float) pose.y,
        (float) pose.z
    );
    msr::airlib::Quaternionr vehicle_orientation(
        (float) pose.w,
        (float) pose.xi,
        (float) -pose.yj,
        (float) -pose.zk
    );
    
    this->airsim_client->simSetVehiclePose(
        msr::airlib::Pose(vehicle_position, vehicle_orientation),
        true,
        vehicle_id
    );
};