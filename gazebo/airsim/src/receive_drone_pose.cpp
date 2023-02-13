#include <thread>

#include "../../src/pose.hpp"
#include "../../src/udp_receiver.hpp"


int main(int argc, char** argv) {
    // TODO - accept a -p input for port specification
    unsigned short int listener_port = 50000;

    UDPReceiver udp_receiver(listener_port);
    PoseTransfer::PoseMessage pose_message;

    std::thread thread_receiver(&UDPReceiver::listen_pose_message, &udp_receiver, &pose_message);

    // TODO - create a simple pose printer to run in separate thread and validate messages updating

    thread_receiver.join();

}