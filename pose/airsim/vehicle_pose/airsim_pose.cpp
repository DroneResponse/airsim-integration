#include "airsim_pose.hpp"

using namespace SimulatorInterface;


AirSimPose::AirSimPose(void *sim_client) : VehiclePose(sim_client) {
    // may not need sim_client if sim specific client type cast works
    this->sim_client = sim_client;
    // type case generic client pointer in AirSim specific client pointer
    this->airsim_client = (msr::airlib::MultirotorRpcLibClient*) sim_client;
};


AirSimPose::~AirSimPose() {};


void AirSimPose::spawn_vehicle(std::string vehicle_id, PoseTransfer::Pose pose) {
    float offset_increment = 3.0;

    msr::airlib::Vector3r vehicle_position(
        (float) pose.x,
        (float) pose.y + offset_increment * (std::stof(vehicle_id) + 1.0),
        (float) pose.z
    );
    msr::airlib::Quaternionr vehicle_orientation(
        (float) pose.w,
        (float) pose.xi,
        (float) -pose.yj,
        (float) -pose.zk
    );

    this->airsim_client->simAddVehicle(
        "drone_" + vehicle_id,
        "SimpleFlight",
        msr::airlib::Pose(vehicle_position, vehicle_orientation)
    );
};


void AirSimPose::set_vehicle_pose(PoseTransfer::Pose pose, std::string vehicle_id) {
    msr::airlib::Vector3r vehicle_position(
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