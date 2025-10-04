#include <string>

#include "gazebo_drone_pose.hpp"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

constexpr int NWIDTH = 7;
static constexpr int MESSAGE_THROTTLE = 100;

GenerateCbLocalPose::GenerateCbLocalPose(
    PoseSender *poseSender)
{
    this->poseSender = poseSender;
    this->poseSender->create_socket();
}

GenerateCbLocalPose::~GenerateCbLocalPose() {};

void GenerateCbLocalPose::trackDroneIds(const std::string droneName)
{
    if (this->droneIds.contains(droneName))
    {
        return;
    }

    const std::string gidPrefix = "_gid_"; // Example droneName: drone_0_gid_123
    size_t gidPrefixPos = droneName.rfind(gidPrefix);

    if (gidPrefixPos != std::string::npos && gidPrefixPos != 0)
    {
        std::string gidStr = droneName.substr(gidPrefixPos + gidPrefix.length());

        try
        {
            // int globalId = std::stoi(gidStr);
            this->droneIds.emplace(droneName, std::stoi(gidStr));
        }
        catch (const std::invalid_argument &e)
        {
            this->droneIds.emplace(droneName, this->uniqueDroneCount);
        }
    }
    else
    {
        this->droneIds.emplace(droneName, this->uniqueDroneCount);
    }

    ++this->uniqueDroneCount;
}

std::string GenerateCbLocalPose::getCurrentTimeInFormat()
{
    auto now = std::chrono::system_clock::now();

    // Convert to time_t to extract date and time components
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime = *std::localtime(&currentTime);

    // Extract fractional seconds
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
                            now.time_since_epoch()) %
                        1'000'000;

    // Use a stringstream to format the output as a string
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
        << std::setw(6) << std::setfill('0') << microseconds.count();

    return oss.str();
}

void GenerateCbLocalPose::resetPoseToDefault(PoseTransfer::Pose &pose)
{
    pose.x = -1.0;
    pose.y = -1.0;
    pose.z = -1.0;
    pose.w = -1.0;
    pose.xi = -1.0;
    pose.yj = -1.0;
    pose.zk = -1.0;
}

void GenerateCbLocalPose::cbLocalPose(ConstPosesStampedPtr &msg)
{
    // "~/pose/local/info" is published at 250 Hz
    std::cout << std::fixed;
    std::cout << std::setprecision(3);
    static int count = 0;

    PoseTransfer::Pose drone_pose = {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0};
    PoseTransfer::Pose camera_pose = {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0};
    std::string current_drone_name;

    for (int i = 0; i < msg->pose_size(); i++)
    {

        current_drone_name = msg->pose(i).name();
        // https://en.cppreference.com/w/cpp/string/basic_string/npos
        // done body pose has no '::' delimiter - drone name only
        const auto &msg_pose_gazebo = msg->pose(i);
        if (current_drone_name.find("::") == std::string::npos)
        {
            current_drone_name = msg_pose_gazebo.name();
            this->trackDroneIds(current_drone_name);
            drone_pose.x = msg_pose_gazebo.position().x();
            drone_pose.y = msg_pose_gazebo.position().y();
            drone_pose.z = msg_pose_gazebo.position().z();
            drone_pose.w = msg_pose_gazebo.orientation().w();
            drone_pose.xi = msg_pose_gazebo.orientation().x();
            drone_pose.yj = msg_pose_gazebo.orientation().y();
            drone_pose.zk = msg_pose_gazebo.orientation().z();
            // if (count % MESSAGE_THROTTLE == 0) {

            std::cout << "Packet Number: " << count << ", Drone Id: "
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
        else if (
            current_drone_name.substr(current_drone_name.find("::") + 2, std::string::npos) == "cgo3_camera_link")
        {
            camera_pose.x = msg_pose_gazebo.position().x();
            camera_pose.y = msg_pose_gazebo.position().y();
            camera_pose.z = msg_pose_gazebo.position().z();
            camera_pose.w = msg_pose_gazebo.orientation().w();
            camera_pose.xi = msg_pose_gazebo.orientation().x();
            camera_pose.yj = msg_pose_gazebo.orientation().y();
            camera_pose.zk = msg_pose_gazebo.orientation().z();
        }

        // 0 doesn't work because initial state is zero for each drone, so using -1.0
        // there may be a better value than -1.0, but even in the off chance xi for either the
        // camera or drone is exactly -1.0, it will likely only be so momentarily
        if (
                (drone_pose.x != -1.0 || camera_pose.x != -1.0) ||
                (drone_pose.y != -1.0 || camera_pose.y != -1.0) ||
                (drone_pose.z != -1.0 || camera_pose.z != -1.0) ||
                (drone_pose.w != -1.0 || camera_pose.w != -1.0) ||
                (drone_pose.xi != -1.0 || camera_pose.xi != -1.0) ||
                (drone_pose.yj != -1.0 || camera_pose.yj != -1.0) ||
                (drone_pose.zk != -1.0 || camera_pose.zk != -1.0) 
        )
            {
                PoseTransfer::PoseMessage pose_message{
                    .message_counter = (uint64_t)count,
                    .drone = drone_pose,
                    .camera = camera_pose,
                    .drone_id = this->droneIds[current_drone_name]};

                // since all poses are grouped together for each drone within a message,
                // reset camera_pose and drone_pose to default values after sending a message
                // all drone a poses, then all drone b poses, then all drone c poses, . . .
                this->poseSender->send_pose_message(pose_message);
                // if (count % MESSAGE_THROTTLE == 0) {
                //     std::cout << "Sent pose for drone id: " << pose_message.drone_id << std::endl;
                // }

                // reset the pose to default values for next drone in message
                this->resetPoseToDefault(drone_pose);
                this->resetPoseToDefault(camera_pose);
            }
    }

    ++count;
}

gazebo::transport::SubscriberPtr GenerateCbLocalPose::subscribeGazeboNode(
    gazebo::transport::NodePtr gazeboNodePtr)
{
    // Listen to Gazebo topics
    // update freq ~250 hz
    return gazeboNodePtr->Subscribe(
        "~/pose/local/info",
        &GenerateCbLocalPose::cbLocalPose,
        this);
}