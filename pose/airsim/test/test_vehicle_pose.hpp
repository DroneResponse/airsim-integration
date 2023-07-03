#include "gmock/gmock.h"

#include "vehicle_pose.hpp"

using namespace SimulatorInterface;

#ifndef TEST_VEHICLE_POSE_H
#define TEST_VEHICLE_POSE_H


class MockSimClient {
    public:
        MockSimClient() {};
};


static MockSimClient mock_sim_client;

class MockVehiclePose : public VehiclePose {
    public:
        MockVehiclePose() : VehiclePose(&mock_sim_client) {};
        MOCK_METHOD(void, spawn_vehicle, (std::string vehicle_id));
        MOCK_METHOD(void, set_vehicle_pose, (PoseTransfer::Pose pose, std::string vehicle_id));
};

#endif