#include <thread>
#include <mutex>

#include "../../src/pose.hpp"
#include "../../src/udp_receiver.hpp"

void print_pose(PoseTransfer::PoseMessage pose_message, std::mutex mutex_pose_message) {
    while (1) {
        mutex_pose_message.lock();
        (std::cout << "drone position: " << pose_message.drone.x << ", " << pose_message.drone.y << ", "
        << pose_message.drone.z << std::endl);
        mutex_pose_message.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


int main(int argc, char** argv) {
    // TODO - accept a -p input for port specification
    unsigned short int listener_port = 50000;

    UDPReceiver udp_receiver(listener_port);
    PoseTransfer::PoseMessage pose_message;

    std::mutex mutex_pose_message;

    std::thread thread_receiver(
        &UDPReceiver::listen_pose_message,
        &udp_receiver,
        &pose_message,
        mutex_pose_message
    );
    std::thread thread_print_pose(print_pose, pose_message);

    thread_receiver.join();
    thread_print_pose.join();
}