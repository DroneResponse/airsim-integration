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