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

#include "airsim_pose.hpp"
#include "pose.hpp"
#include "udp_receiver.hpp"
#include "pose_handlers.hpp"


/**
 * prints pose messages every 500 ms
 * @param pose_message reference to a pose message
 * @param mutex_pose_message mutex to lock access to provided pose message when reading
*/
void print_pose(PoseTransfer::PoseMessage &pose_message, std::mutex &mutex_pose_message) {
    while (1) {
        mutex_pose_message.lock();
        (std::cout << "drone position received: " << pose_message.drone.x << ", " << pose_message.drone.y
        << ", " << pose_message.drone.z << std::endl);
        (std::cout << "drone orientation received: " << pose_message.drone.w << ", "
        << pose_message.drone.xi << ", " << pose_message.drone.yj << ", "
        << pose_message.drone.zk << std::endl);
        mutex_pose_message.unlock();
        (std::cout << "camera orientation received: " << pose_message.camera.w << ", "
        << pose_message.camera.xi << ", " << pose_message.camera.yj << ", "
        << pose_message.camera.zk << "\n" << std::endl);
        mutex_pose_message.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


/**
 * converts gimbal mounted camera local orientation to a global orientation. 
 * @param camDroneOrientation camera local quaternion where x = roll, y = pitch, z = yaw
 * @param droneOrientation drone global quaternion where x = roll, y = pitch, z = yaw
 * @return camera global quaternion where x = roll, y = pitch, z = yaw
 */
static msr::airlib::Quaternionr cam_drone_to_global(
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
static msr::airlib::Quaternionr remove_roll(msr::airlib::Quaternionr orientation) {
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
 * sets drone pose in airsim to gazebo gimbal camera pose
 * @param airsim_client reference to airsim client
 * @param pose_message reference to a pose message
 * @param mutex_pose_message mutex to lock access to provided pose message when reading
 * @param roll_override overrides roll if set to true
*/
void set_drone_pose_as_camera_pose(
    msr::airlib::MultirotorRpcLibClient &airsim_client,
    PoseTransfer::PoseMessage &pose_message,
    std::mutex &mutex_pose_message,
    bool roll_override
) {
    uint64_t msg_count = 0;
    while(1) {
        mutex_pose_message.lock();
        if (pose_message.message_counter > msg_count) {
            msr::airlib::Vector3r drone_position(
                (float) pose_message.drone.x,
                (float) -pose_message.drone.y,
                (float) -pose_message.drone.z
            );
            msr::airlib::Quaternionr drone_orientation(
                (float) pose_message.drone.w,
                (float) pose_message.drone.xi,
                (float) -pose_message.drone.yj,
                (float) -pose_message.drone.zk
            );
            msr::airlib::Quaternionr camera_drone_orientation(
                (float) pose_message.camera.w,
                (float) pose_message.camera.xi,
                (float) -pose_message.camera.yj,
                (float) -pose_message.camera.zk
            );

            if (roll_override) {
                msr::airlib::Quaternionr camera_drone_orientation = remove_roll(
                    msr::airlib::Quaternionr(
                        (float) pose_message.camera.w,
                        (float) pose_message.camera.xi,
                        (float) -pose_message.camera.yj,
                        (float) -pose_message.camera.zk
                    )
                );
            }
            
            msr::airlib::Quaternionr camera_global_orientation = cam_drone_to_global(
                camera_drone_orientation,
                drone_orientation
            );

            airsim_client.simSetVehiclePose(
                msr::airlib::Pose(drone_position, camera_global_orientation),
                true
            );
        }
        msg_count = pose_message.message_counter;
        mutex_pose_message.unlock();
        // update at ~200hz
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}


int main(int argc, char** argv) {
    unsigned short int listener_port = 50000;
    bool roll_override = false;
    void (*drone_pose_setter)(
        SimulatorInterface::VehiclePose*,
        PoseTransfer::PoseMessage*,
        std::mutex*,
        bool*
    ) = &PoseHandlers::set_drone_pose;

    for (int i=0; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> listener_port)) {
                std::cerr << "Invalid port: " << argv[i + 1] << '\n';
            } else if (!ss.eof()) {
                std::cerr << "Trailing characters after port: " << argv[i + 1] << '\n';
            }
        }
        // if (strcmp(argv[i], "-c") == 0) {
        //     drone_pose_setter = &set_drone_pose_as_camera_pose;
        // }
        if (strcmp(argv[i], "-r") == 0) {
            roll_override = true;
        }
    }

    // if (drone_pose_setter == &set_drone_pose_as_camera_pose) {
    //     std::cout << "Drone pose will be set to global gimbal camera pose\n";
    // } else {
    //     std::cout << "Drone pose will be set to global drone pose\n";
    // }

    if (roll_override) {
        std::cout << "Camera roll will be artifically removed\n";
    }

    std::cout << "Listening for pose messages on port " << listener_port << std::endl;

    msr::airlib::MultirotorRpcLibClient airsim_client;
    airsim_client.confirmConnection();

    SimulatorInterface::AirSimPose vehicle_interface(&airsim_client);

    UDPReceiver udp_receiver(listener_port);
    PoseTransfer::PoseMessage pose_message;
    memset(&pose_message, 0, sizeof(pose_message));

    std::mutex mutex_pose_message;

    // could change to false to stop all threads with certain signals rather than ctrl+c?
    bool continue_processing_messages = true;

    // pass by ref inputs must be wrapped in std::ref() within thread constructor
    std::thread thread_receiver(
        &UDPReceiver::listen_pose_message,
        &udp_receiver,
        &pose_message,
        std::ref(mutex_pose_message)
    );
    std::thread thread_print_pose(print_pose, std::ref(pose_message), std::ref(mutex_pose_message));
    std::thread thread_set_airsim_pose(
        *drone_pose_setter,
        &vehicle_interface,
        &pose_message,
        &mutex_pose_message,
        &continue_processing_messages
    );

    thread_receiver.join();
    thread_print_pose.join();
    thread_set_airsim_pose.join();
}