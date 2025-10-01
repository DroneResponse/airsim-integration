// Started with GazeboDrone in the Microsoft AirSim repo: https://github.com/Microsoft/AirSim.git
#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/gazebo_client.hh>

#include <chrono>
#include <iostream>
#include <sstream>

#include "udp_sender.hpp"
#include "gazebo_drone_pose.hpp"


int main(int argc, char** argv)
{
    std::string AIRSIM_HOST = "127.0.0.1";
    unsigned short AIRSIM_PORT = 50000;

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
            AIRSIM_HOST = ss;
        }
    }

    UDPSender udpSender (AIRSIM_HOST, AIRSIM_PORT);
    GenerateCbLocalPose generateCbLocalPose (&udpSender);

    // print out the version of gazebo
    std::cout << "Gazebo version: " << GAZEBO_MAJOR_VERSION << "." << GAZEBO_MINOR_VERSION << std::endl;
    std::cout << "Sending pose messages to " << AIRSIM_HOST << " on port " << AIRSIM_PORT << std::endl;

    // Load gazebo
    gazebo::client::setup(argc, argv);

    // Create our node for communication
    gazebo::transport::NodePtr gazeboNodePtr(new gazebo::transport::Node());
    gazeboNodePtr->Init();

    
    gazebo::transport::SubscriberPtr sub_pose1 = generateCbLocalPose.subscribeGazeboNode(gazeboNodePtr);

    while (true)
        gazebo::common::Time::MSleep(10);

    // Make sure to shut everything down.
    gazebo::client::shutdown();
}
