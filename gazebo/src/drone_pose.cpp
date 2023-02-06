/*
 * Copyright (C) 2012 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// Taken from the Microsoft AirSim repo: https://github.com/Microsoft/AirSim.git

#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/gazebo_client.hh>
#include "common/common_utils/StrictMode.hpp"
STRICT_MODE_OFF
#ifndef RPCLIB_MSGPACK
#define RPCLIB_MSGPACK clmdep_msgpack
#endif // !RPCLIB_MSGPACK
#include "rpc/rpc_error.h"
STRICT_MODE_ON

#include "vehicles/multirotor/api/MultirotorRpcLibClient.hpp"
#include "Eigen/Dense"
#include <chrono>

#include <iostream>

constexpr int NWIDTH = 7;
static constexpr int MESSAGE_THROTTLE = 100;

using namespace msr::airlib;

msr::airlib::MultirotorRpcLibClient client;
std::vector<std::string> vehicleList;
bool rollOverride = false;


/**
 * local pose callback where gazebo drone represents airsim drone's global pose
 * @param msg gazebo message
*/
void cbLocalPose(ConstPosesStampedPtr& msg)
{
    std::cout << std::fixed;
    std::cout << std::setprecision(3);
    static int count = 0;

    if (count % MESSAGE_THROTTLE == 0) {
        std::cout << "The number of vehicles is: " << vehicleList.size();
        std::cout << "\n";

        std::cout << "The first vehicle is: " << vehicleList[0];
        std::cout << "\n" << std::endl;
    }

    for (int i = 0; i < msg->pose_size(); i++) {
        auto x = msg->pose(i).position().x();
        auto y = msg->pose(i).position().y();
        auto z = msg->pose(i).position().z();
        auto ow = msg->pose(i).orientation().w();
        auto ox = msg->pose(i).orientation().x();
        auto oy = msg->pose(i).orientation().y();
        auto oz = msg->pose(i).orientation().z();
        if (count % MESSAGE_THROTTLE == 0) {
            std::cout << "local (" << std::setw(2) << i << ") ";
            std::cout << std::left << std::setw(32) << msg->pose(i).name();
            std::cout << " x: " << std::right << std::setw(NWIDTH) << x;
            std::cout << " y: " << std::right << std::setw(NWIDTH) << y;
            std::cout << " z: " << std::right << std::setw(NWIDTH) << z;

            std::cout << " ow: " << std::right << std::setw(NWIDTH) << ow;
            std::cout << " ox: " << std::right << std::setw(NWIDTH) << ox;
            std::cout << " oy: " << std::right << std::setw(NWIDTH) << oy;
            std::cout << " oz: " << std::right << std::setw(NWIDTH) << oz;
            std::cout << std::endl;
        }
        if (i == 0) {
            msr::airlib::Vector3r p(x, -y, -z);
            msr::airlib::Quaternionr o(ow, ox, -oy, -oz);

            client.simSetVehiclePose(Pose(p, o), true);
        }
    }
    if (count % MESSAGE_THROTTLE == 0) {
        std::cout << std::endl;
    }

    ++count;
}


/**
 * converts gimbal mounted camera local orientation to a global orientation. 
 * @param camDroneOrientation camera local quaternion where x = roll, y = pitch, z = yaw
 * @param droneOrienation drone global quaternion where x = roll, y = pitch, z = yaw
 * @return camera global quaternion where x = roll, y = pitch, z = yaw
 */
static msr::airlib::Quaternionr camDroneToGlobal(
    msr::airlib::Quaternionr camDroneOrientation,
    msr::airlib::Quaternionr droneOrientation
) {
    // need to remove pitch and roll from drone orientation since camera pitch and roll
    // are already in the global frame
    float drone_yaw = msr::airlib::VectorMath::getYaw(droneOrientation);
    msr::airlib::Quaternionr drone_orientation_yaw_only(
        std::cos(drone_yaw / 2.0),
        0,
        0,
        std::sin(drone_yaw / 2.0));

    return drone_orientation_yaw_only * camDroneOrientation;
}


/**
 * removes roll (y) from the provided quaternion.
 * @param orientation quaternion where x = roll, y = pitch, z = yaw
 * @return quaternion where x = roll, y = pitch, z = yaw
 */
static msr::airlib::Quaternionr removeRoll(msr::airlib::Quaternionr orientation) {
    float pitch = msr::airlib::VectorMath::getPitch(orientation);
    float yaw = msr::airlib::VectorMath::getYaw(orientation);

    msr::airlib::Quaternionr orientation_pitch_only(
        std::cos(pitch / 2.0),
        0,
        std::sin(pitch / 2.0),
        0);

    msr::airlib::Quaternionr orientation_yaw_only(
        std::cos(yaw / 2.0),
        0,
        0,
        std::sin(yaw / 2.0));

    // follow assumed previous orientation assembly with roll -> pitch -> yaw
    return orientation_yaw_only * orientation_pitch_only;
}


/**
 * local pose callback where gazebo typhoon_h480 gimbal camera represents airsim drone's global pose
 * @param msg gazebo message
*/
void cbDroneAsCameraPose(ConstPosesStampedPtr& msg)
{
    std::cout << std::fixed;
    std::cout << std::setprecision(3);
    static int count = 0;

    // initialize with nanf
    msr::airlib::Quaternionr drone_world_o(std::nanf(""), std::nanf(""), std::nanf(""), std::nanf(""));
    msr::airlib::Quaternionr cam_drone_o(std::nanf(""), std::nanf(""), std::nanf(""), std::nanf(""));
    msr::airlib::Quaternionr cam_global_o(std::nanf(""), std::nanf(""), std::nanf(""), std::nanf(""));
    msr::airlib::Vector3r p(std::nanf(""), std::nanf(""), std::nanf(""));
    
    if (count % MESSAGE_THROTTLE == 0) {
        std::cout << "The number of vehicles is: " << vehicleList.size();
        std::cout << "\n";

        std::cout << "The first vehicle is: " << vehicleList[0];
        std::cout << "\n" << std::endl;
    }

    for (int i = 0; i < msg->pose_size(); i++) {
        auto x = msg->pose(i).position().x();
        auto y = msg->pose(i).position().y();
        auto z = msg->pose(i).position().z();
        auto ow = msg->pose(i).orientation().w();
        auto ox = msg->pose(i).orientation().x();
        auto oy = msg->pose(i).orientation().y();
        auto oz = msg->pose(i).orientation().z();
        if (count % MESSAGE_THROTTLE == 0) {
            std::cout << "local (" << std::setw(2) << i << ") ";
            std::cout << std::left << std::setw(32) << msg->pose(i).name();
            std::cout << " x: " << std::right << std::setw(NWIDTH) << x;
            std::cout << " y: " << std::right << std::setw(NWIDTH) << y;
            std::cout << " z: " << std::right << std::setw(NWIDTH) << z;

            std::cout << " ow: " << std::right << std::setw(NWIDTH) << ow;
            std::cout << " ox: " << std::right << std::setw(NWIDTH) << ox;
            std::cout << " oy: " << std::right << std::setw(NWIDTH) << oy;
            std::cout << " oz: " << std::right << std::setw(NWIDTH) << oz;
            std::cout << std::endl;
        }
        // update freq ~250 hz
        if (i == 0) {
            // drone position and orientaiton in global frame
            p = msr::airlib::Vector3r(x, -y, -z);
            drone_world_o = msr::airlib::Quaternionr(ow, ox, -oy, -oz);
            if (count % MESSAGE_THROTTLE == 0) {
                std::cout << "Camera drone quaternion (xyzw): \n" << cam_drone_o.coeffs() << std::endl;
                std::cout << "Drone world quaternion (xyzw): \n" << drone_world_o.coeffs() << std::endl;
            }
        }
        // set drone attitude from camera attitude
        if (msg->pose(i).name() == "typhoon_h480::cgo3_camera_link") {
            // orientation of the camera in drone's reference frame
            if (rollOverride) {
                cam_drone_o = removeRoll(msr::airlib::Quaternionr(ow, ox, -oy, -oz));
            } else {
                cam_drone_o = msr::airlib::Quaternionr(ow, ox, -oy, -oz);
            }
            if (count % MESSAGE_THROTTLE == 0) {
                std::cout << "Camera drone quaternion (xyzw): \n" << cam_drone_o.coeffs() << std::endl;
                std::cout << "Drone world quaternion (xyzw): \n" << drone_world_o.coeffs() << std::endl;
            }
        }

        cam_global_o = camDroneToGlobal(cam_drone_o, drone_world_o);
        // TODO: loop through vehicles for multidrone sim
        client.simSetVehiclePose(Pose(p, cam_global_o), true, vehicleList[0]);
    }
    if (count % MESSAGE_THROTTLE == 0) {
        std::cout << std::endl;
    }

    ++count;
}


/**
 * global pose callback that simply prints global poses
 * @param msg gazebo message
*/
void cbGlobalPose(ConstPosesStampedPtr& msg)
{
    std::cout << std::fixed;
    std::cout << std::setprecision(4);
    static int count = 0;
    if (count % MESSAGE_THROTTLE) {
        ++count;
        return;
    }
    ++count;

    for (int i = 0; i < msg->pose_size(); i++) {
        std::cout << "global (" << i << ") ";
        std::cout << std::left << std::setw(32) << msg->pose(i).name();
        std::cout << " x: " << std::right << std::setfill(' ') << std::setw(NWIDTH) << msg->pose(i).position().x();
        std::cout << " y: " << std::right << std::setfill(' ') << std::setw(NWIDTH) << msg->pose(i).position().y();
        std::cout << " z: " << std::right << std::setfill(' ') << std::setw(NWIDTH) << msg->pose(i).position().z();

        std::cout << " ow: " << std::right << std::setfill(' ') << std::setw(NWIDTH) << msg->pose(i).orientation().w();
        std::cout << " ox: " << std::right << std::setfill(' ') << std::setw(NWIDTH) << msg->pose(i).orientation().x();
        std::cout << " oy: " << std::right << std::setfill(' ') << std::setw(NWIDTH) << msg->pose(i).orientation().y();
        std::cout << " oz: " << std::right << std::setfill(' ') << std::setw(NWIDTH) << msg->pose(i).orientation().z();
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int main(int argc, char** argv)
{
    void (*localPoseCallback)(ConstPosesStampedPtr&) = &cbLocalPose;
    for (int i=0; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            localPoseCallback = &cbDroneAsCameraPose;
        }
        if (strcmp(argv[i], "-r") == 0) {
            rollOverride = true;
        }
    }

    if (localPoseCallback == &cbDroneAsCameraPose) {
        std::cout << "Drone pose will be set to global gimbal camera pose\n";
    } else {
        std::cout << "Drone pose will be set to global drone pose\n";
    }

    if (rollOverride) {
        std::cout << "Camera roll will be artifically removed\n";
    }

    client.confirmConnection();
    // don't want to call on every message because blocks for too long
    vehicleList = client.listVehicles();

    // Load gazebo
    gazebo::client::setup(argc, argv);

    // Create our node for communication
    gazebo::transport::NodePtr node(new gazebo::transport::Node());
    node->Init();

    // Listen to Gazebo topics
    // update freq ~250 hz
    gazebo::transport::SubscriberPtr sub_pose1 = node->Subscribe("~/pose/local/info", *localPoseCallback);
    // update freq ~50 hz
    gazebo::transport::SubscriberPtr sub_pose2 = node->Subscribe("~/pose/info", cbGlobalPose);

    while (true)
        gazebo::common::Time::MSleep(10);

    // Make sure to shut everything down.
    gazebo::client::shutdown();
}
