// Started with GazeboDrone in the Microsoft AirSim repo: https://github.com/Microsoft/AirSim.git
#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/gazebo_client.hh>

#include <chrono>
#include <iostream>
#include <sstream>

#include "pose.hpp"
#include "udp_sender.hpp"

constexpr int NWIDTH = 7;
static constexpr int MESSAGE_THROTTLE = 100;

// TODO - use function generator to set host and port in gazebo callbacks instead of globals here
std::string AIRSIM_HOST = "127.0.0.1";
unsigned short AIRSIM_PORT = 50000;

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
    static UDPSender udp_sender (AIRSIM_HOST, AIRSIM_PORT);

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
            .message_counter = (uint64_t) count,
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


int main(int argc, char** argv)
{
    for (int i=0; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> AIRSIM_PORT)) {
                std::cerr << "Invalid port: " << argv[i + 1] << '\n';
            } else if (!ss.eof()) {
                std::cerr << "Trailing characters after port: " << argv[i + 1] << '\n';
            }
        }
        if (strcmp(argv[i], "-a") == 0) {
            std::string ss = argv[i + 1];
            if (strcmp(&ss.back(), " ") == 0) {
                std::cerr << "Trailing spaces after address: " << ss << '\n';
            } else if (!(std::count(ss.begin(), ss.end(), '.') == 3)) {
                std::cerr << "Invalid address format: " << ss << '\n';
            } else {
                AIRSIM_HOST = ss;
            }
        }
    }

    // print out the version of gazebo
    std::cout << "Gazebo version: " << GAZEBO_MAJOR_VERSION << "." << GAZEBO_MINOR_VERSION << std::endl;
    std::cout << "Sending pose messages to " << AIRSIM_HOST << " on port " << AIRSIM_PORT << std::endl;

    // Load gazebo
    gazebo::client::setup(argc, argv);

    // Create our node for communication
    gazebo::transport::NodePtr node(new gazebo::transport::Node());
    node->Init();

    // Listen to Gazebo topics
    // update freq ~250 hz
    gazebo::transport::SubscriberPtr sub_pose1 = node->Subscribe("~/pose/local/info", cbLocalPose);

    while (true)
        gazebo::common::Time::MSleep(10);

    // Make sure to shut everything down.
    gazebo::client::shutdown();
}
