#include "gazebo_drone_pose.hpp"
#include "pose.hpp"
#include "udp_sender.hpp"

constexpr int NWIDTH = 7;
static constexpr int MESSAGE_THROTTLE = 100;


GenerateCbLocalPose::GenerateCbLocalPose(UDPSender* udpSender) {
    this->udpSender = udpSender;
}


GenerateCbLocalPose::~GenerateCbLocalPose() {};


void GenerateCbLocalPose::cbLocalPose(ConstPosesStampedPtr& msg) {
    // "~/pose/local/info" is published at 250 Hz
    std::cout << std::fixed;
    std::cout << std::setprecision(3);
    static int count = 0;

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
        this->udpSender->send_pose_message(pose_message);
    }

    if (count % MESSAGE_THROTTLE == 0) {
        std::cout << std::endl;
    }

    ++count;
}