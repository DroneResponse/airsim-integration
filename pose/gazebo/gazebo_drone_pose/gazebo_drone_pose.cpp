#include <string>

#include "gazebo_drone_pose.hpp"
#include "pose.hpp"
#include "pose_sender.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

constexpr int NWIDTH = 7;
static constexpr int MESSAGE_THROTTLE = 100;

// Sentinel value for positions and orientations.
// Zero doesn't work because that's the initial state for each drone
// In the off chance xi for either the camera or drone is exactly NOT_SET,
// it will likely only be so momentarily.
#define NOT_SET -1.0

GenerateCbLocalPose::GenerateCbLocalPose(
    PoseSender *poseSender
) {
    this->poseSender = poseSender;
    this->poseSender->create_socket();
}

GenerateCbLocalPose::~GenerateCbLocalPose() {};

void GenerateCbLocalPose::trackDroneIds(std::string droneName) {
    if (this->droneIds.contains(droneName)) {
        return;
    }

    const std::string gidPrefix = "_gid_"; // Example droneName: drone_0_gid_123
    size_t gidPrefixPos = droneName.rfind(gidPrefix);

    if (gidPrefixPos != std::string::npos && gidPrefixPos != 0) {
        std::string gidStr = droneName.substr(gidPrefixPos + gidPrefix.length());

        try {
            int globalId = std::stoi(gidStr);
            this->droneIds[droneName] = globalId;

        } catch (const std::invalid_argument &e) {
            this->droneIds[droneName] = uniqueDroneCount;
        }
    } else {
        this->droneIds[droneName] = uniqueDroneCount;
    }

    this->uniqueDroneCount++;
}

std::string GenerateCbLocalPose::getCurrentTimeInFormat() {
    auto now = std::chrono::system_clock::now();

    // Convert to time_t to extract date and time components
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime = *std::localtime(&currentTime);

    // Extract fractional seconds
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()) % 1'000'000;

    // Use a stringstream to format the output as a string
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
        << std::setw(6) << std::setfill('0') << microseconds.count();

    return oss.str();
}

void GenerateCbLocalPose::cbLocalPose(ConstPosesStampedPtr &msg) {
    // "~/pose/local/info" is published at 250 Hz
    std::cout << std::fixed;
    std::cout << std::setprecision(3);
    static int count = 0;

    PoseTransfer::Pose drone_pose = (PoseTransfer::Pose){
        .x = NOT_SET,
        .y = NOT_SET,
        .z = NOT_SET,
        .w = NOT_SET,
        .xi = NOT_SET,
        .yj = NOT_SET,
        .zk = NOT_SET,
    };
    PoseTransfer::Pose camera_pose = (PoseTransfer::Pose){
        .x = NOT_SET,
        .y = NOT_SET,
        .z = NOT_SET,
        .w = NOT_SET,
        .xi = NOT_SET,
        .yj = NOT_SET,
        .zk = NOT_SET,
    };
    std::string current_drone_name;

    for (int i = 0; i < msg->pose_size(); i++) {
        auto x = msg->pose(i).position().x();
        auto y = msg->pose(i).position().y();
        auto z = msg->pose(i).position().z();
        auto ow = msg->pose(i).orientation().w();
        auto ox = msg->pose(i).orientation().x();
        auto oy = msg->pose(i).orientation().y();
        auto oz = msg->pose(i).orientation().z();

        std::string msg_name = msg->pose(i).name();
        // https://en.cppreference.com/w/cpp/string/basic_string/npos
        // drone body pose has no '::' delimiter - drone name only
        if (msg_name.find("::") == std::string::npos) {
            current_drone_name = msg->pose(i).name();
            this->trackDroneIds(current_drone_name);
            drone_pose.x = x;
            drone_pose.y = y;
            drone_pose.z = z;
            drone_pose.w = ow;
            drone_pose.xi = ox;
            drone_pose.yj = oy;
            drone_pose.zk = oz;

            // std::cout << "Packet Number: " << count << ", Drone Id: "
            //           << std::to_string(this->droneIds[current_drone_name]) << ", Timestamp: "
            //           << this->getCurrentTimeInFormat() << " , Drone Position: "
            //           << std::to_string(drone_pose.x) << ", "
            //           << std::to_string(drone_pose.y) << ", "
            //           << std::to_string(drone_pose.z) << ", Drone Orientation: "
            //           << std::to_string(drone_pose.w) << ", "
            //           << std::to_string(drone_pose.xi) << ", "
            //           << std::to_string(drone_pose.yj) << ", "
            //           << std::to_string(drone_pose.zk)
            //           << std::endl;

        } else if (msg_name.substr(msg_name.find("::") + 2, std::string::npos) == "cgo3_camera_link") {

            camera_pose.x = x;
            camera_pose.y = y;
            camera_pose.z = z;
            camera_pose.w = ow;
            camera_pose.xi = ox;
            camera_pose.yj = oy;
            camera_pose.zk = oz;
        }

        if (drone_pose.xi != NOT_SET) {
            PoseTransfer::PoseMessage pose_message{
                .message_counter = (uint64_t)count,
                .drone = drone_pose,
                .camera = camera_pose,
                .drone_id = this->droneIds[current_drone_name]};

            // since all poses are grouped together for each drone within a message,
            // reset camera_pose and drone_pose to default values after sending a message
            // all drone a poses, then all drone b poses, then all drone c poses, . . .
            this->poseSender->send_pose_message(pose_message);

            if (count % MESSAGE_THROTTLE == 0) {
                std::cout << "Sent position #" << count << ", Drone Id: "
                          << std::to_string(this->droneIds[current_drone_name]) << ", Timestamp: "
                          << this->getCurrentTimeInFormat() << " , Drone Position: "
                          << std::to_string(drone_pose.x) << ", "
                          << std::to_string(drone_pose.y) << ", "
                          << std::to_string(drone_pose.z) << ", Drone Orientation: "
                          << std::to_string(drone_pose.w) << ", "
                          << std::to_string(drone_pose.xi) << ", "
                          << std::to_string(drone_pose.yj) << ", "
                          << std::to_string(drone_pose.zk)
                          << std::endl;
            }

            reset_vectors(drone_pose, camera_pose);
        }
    }

    ++count;
}

// Reset the pose vectors to NOT_SET values.
void GenerateCbLocalPose::reset_vectors(PoseTransfer::Pose &drone_pose, PoseTransfer::Pose &camera_pose) {
    drone_pose.x = NOT_SET;
    drone_pose.y = NOT_SET;
    drone_pose.z = NOT_SET;
    drone_pose.w = NOT_SET;
    drone_pose.xi = NOT_SET;
    drone_pose.yj = NOT_SET;
    drone_pose.zk = NOT_SET;

    camera_pose.x = NOT_SET;
    camera_pose.y = NOT_SET;
    camera_pose.z = NOT_SET;
    camera_pose.w = NOT_SET;
    camera_pose.xi = NOT_SET;
    camera_pose.yj = NOT_SET;
    camera_pose.zk = NOT_SET;
}

gazebo::transport::SubscriberPtr GenerateCbLocalPose::subscribeGazeboNode(
    gazebo::transport::NodePtr gazeboNodePtr
) {
    // Listen to Gazebo topics
    // update freq ~250 hz
    return gazeboNodePtr->Subscribe(
        "~/pose/local/info",
        &GenerateCbLocalPose::cbLocalPose,
        this
    );
}
