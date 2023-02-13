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

#include <chrono>
#include <iostream>

#include "../../src/pose.hpp"
#include "../../src/udp_sender.hpp"

constexpr int NWIDTH = 7;
static constexpr int MESSAGE_THROTTLE = 100;

// TODO - use function generator to set host and port in gazebo callbacks instead of globals here
std::string airsim_host = "127.0.0.1";
unsigned short airsim_port = 50000;

/**
 * local pose callback where gazebo drone represents airsim drone's global pose
 * @param msg gazebo message
*/
void cbLocalPose(ConstPosesStampedPtr& msg)
{
    // "~/pose/local/info" is published at 250 Hz
    std::cout << std::fixed;
    std::cout << std::setprecision(3);
    static int count = 0;
    static const UDPSender::UDPSender(airsim_host, airsim_port) udp_sender;

    PoseTransfer::Pose drone_pose;
    PoseTransfer::Pose camera_pose;
    memset(&drone_pose, 0, sizeof(drone_pose));
    memset(&camera_pose, 0, sizeof(camera_pose));

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
            drone_pose.x = x;
            drone_pose.y = y;
            drone_pose.z = z;
            drone_pose.w = ow;
            drone_pose.xi = ox;
            drone_pose.yj = oy;
            drone_pose.zk = oz;
        }
        if (msg->pose(i).name() == "typhoon_h480::cgo3_camera_link") {
            camera_pose.x = x;
            camera_pose.y = y;
            camera_pose.z = z;
            camera_pose.w = ow;
            camera_pose.xi = ox;
            camera_pose.yj = oy;
            camera_pose.zk = oz;
        }
    }

    if (drone_pose.x != 0 && camera_pose.x != 0) {
        PoseTransfer::PoseMessage pose_message {
            .message_counter = count,
            .drone = drone_pose,
            .camera = camera_pose
        };
        udp_sender.send_pose_message(pose_message);
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


send_pose_message(const PoseMessage& pose_message, socket_t& socket)
{
    // send pose message to airsim
    zmq::message_t message(sizeof(PoseMessage));
    memcpy(message.data(), &pose_message, sizeof(PoseMessage));
    socket.send(message);

}

int main(int argc, char** argv)
{
    std::string air_sim_host = "127.0.0.1";
    int air_sim_port = 41451;
    // print out the version of gazebo
    std::cout << "Gazebo version: " << GAZEBO_MAJOR_VERSION << "." << GAZEBO_MINOR_VERSION << std::endl;

    // Load gazebo
    gazebo::client::setup(argc, argv);

    // Create our node for communication
    gazebo::transport::NodePtr node(new gazebo::transport::Node());
    node->Init();

    // Listen to Gazebo topics
    // update freq ~250 hz
    gazebo::transport::SubscriberPtr sub_pose1 = node->Subscribe("~/pose/local/info", cbLocalPose);
    // update freq ~50 hz
    gazebo::transport::SubscriberPtr sub_pose2 = node->Subscribe("~/pose/info", cbGlobalPose);

    while (true)
        gazebo::common::Time::MSleep(10);

    // Make sure to shut everything down.
    gazebo::client::shutdown();
}
