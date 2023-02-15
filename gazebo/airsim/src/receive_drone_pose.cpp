#include <mutex>
#include <sstream>
#include <thread>

// airsim dependencies
#include "common/common_utils/StrictMode.hpp"
STRICT_MODE_OFF
#ifndef RPCLIB_MSGPACK
#define RPCLIB_MSGPACK clmdep_msgpack
#endif // !RPCLIB_MSGPACK
#include "rpc/rpc_error.h"
STRICT_MODE_ON
#include "vehicles/multirotor/api/MultirotorRpcLibClient.hpp"
#include "Eigen/Dense"

#include "pose.hpp"
#include "udp_receiver.hpp"


void print_pose(PoseTransfer::PoseMessage &pose_message, std::mutex &mutex_pose_message) {
    while (1) {
        mutex_pose_message.lock();
        (std::cout << "drone position: " << pose_message.drone.x << ", " << pose_message.drone.y << ", "
        << pose_message.drone.z << std::endl);
        mutex_pose_message.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


int main(int argc, char** argv) {
    unsigned short int listener_port = 50000;

    for (int i=0; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> listener_port)) {
                std::cerr << "Invalid port: " << argv[i + 1] << '\n';
            } else if (!ss.eof()) {
                std::cerr << "Trailing characters after port: " << argv[i + 1] << '\n';
            }
        }
    }

    std::cout << "Listening for pose messages on port " << listener_port << std::endl;

    UDPReceiver udp_receiver(listener_port);
    PoseTransfer::PoseMessage pose_message;
    memset(&pose_message, 0, sizeof(pose_message));

    std::mutex mutex_pose_message;

    // pass by ref inputs must be wrapped in std::ref() within thread constructor
    std::thread thread_receiver(
        &UDPReceiver::listen_pose_message,
        &udp_receiver,
        &pose_message,
        std::ref(mutex_pose_message)
    );
    std::thread thread_print_pose(print_pose, std::ref(pose_message), std::ref(mutex_pose_message));

    thread_receiver.join();
    thread_print_pose.join();
}