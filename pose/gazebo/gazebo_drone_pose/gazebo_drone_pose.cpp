#include "gazebo_drone_pose.hpp"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

constexpr int NWIDTH = 7;
static constexpr int MESSAGE_THROTTLE = 250;

GenerateCbLocalPose::GenerateCbLocalPose(
    PoseSender *poseSender, std::string drone_name)
{
    this->poseSender = poseSender;
    this->poseSender->create_socket();
    this->drone_name = drone_name;
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

void GenerateCbLocalPose::resetPoseToDefault(uint16_t key)
{
    this->drone_pose_map[key].x = -1;
    this->drone_pose_map[key].y = -1;
    this->drone_pose_map[key].z = -1;
    this->drone_pose_map[key].w = -1;
    this->drone_pose_map[key].xi = -1;
    this->drone_pose_map[key].yj = -1;
    this->drone_pose_map[key].zk = -1;

    this->camera_pose_map[key].x = -1;
    this->camera_pose_map[key].y = -1;
    this->camera_pose_map[key].z = -1;
    this->camera_pose_map[key].w = -1;
    this->camera_pose_map[key].xi = -1;
    this->camera_pose_map[key].yj = -1;
    this->camera_pose_map[key].zk = -1;
}

void GenerateCbLocalPose::cbLocalPose(ConstPosesStampedPtr &msg)
{
    // "~/pose/local/info" is published at 250 Hz
    std::cout << std::fixed;
    std::cout << std::setprecision(3);
    static int count = 0;

    std::string current_drone_name;

    for (int i = 0; i < msg->pose_size(); i++)
    {
        current_drone_name = msg->pose(i).name();
        size_t delimiter_pos = current_drone_name.find("::");
        // https://en.cppreference.com/w/cpp/string/basic_string/npos
        // done body pose has no '::' delimiter - drone name only
        const auto &msg_pose_gazebo = msg->pose(i);
        if (delimiter_pos == std::string::npos && current_drone_name.starts_with(this->drone_name))
        {
            // track the drone id
            this->trackDroneIds(current_drone_name);
            // check if the droneID is already part of cd lthe hashmap
            if(!this->drone_pose_map.contains(this->droneIds[current_drone_name])){
                // add the empty pose here
                PoseTransfer::Pose drone_pose = {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0};
                this->drone_pose_map[this->droneIds[current_drone_name]] = drone_pose;
            }

            this->drone_pose_map[this->droneIds[current_drone_name]].x = msg_pose_gazebo.position().x();
            this->drone_pose_map[this->droneIds[current_drone_name]].y = msg_pose_gazebo.position().y();
            this->drone_pose_map[this->droneIds[current_drone_name]].z = msg_pose_gazebo.position().z();
            this->drone_pose_map[this->droneIds[current_drone_name]].w = msg_pose_gazebo.orientation().w();
            this->drone_pose_map[this->droneIds[current_drone_name]].xi = msg_pose_gazebo.orientation().x();
            this->drone_pose_map[this->droneIds[current_drone_name]].yj = msg_pose_gazebo.orientation().y();
            this->drone_pose_map[this->droneIds[current_drone_name]].zk = msg_pose_gazebo.orientation().z();
        }
        else if (
            current_drone_name.substr(delimiter_pos + 2, std::string::npos) == "cgo3_camera_link")
        {
            // check if the droneID is already part of the hashmap
            std::string drone_name = current_drone_name.substr(0, delimiter_pos);
            if(!this->camera_pose_map.contains(this->droneIds[drone_name])){
                // add the empty pose here
                PoseTransfer::Pose camera_pose = {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0};
                this->camera_pose_map[this->droneIds[drone_name]] = camera_pose;
            }

            this->camera_pose_map[this->droneIds[drone_name]].x = msg_pose_gazebo.position().x();
            this->camera_pose_map[this->droneIds[drone_name]].y = msg_pose_gazebo.position().y();
            this->camera_pose_map[this->droneIds[drone_name]].z = msg_pose_gazebo.position().z();
            this->camera_pose_map[this->droneIds[drone_name]].w = msg_pose_gazebo.orientation().w();
            this->camera_pose_map[this->droneIds[drone_name]].xi = msg_pose_gazebo.orientation().x();
            this->camera_pose_map[this->droneIds[drone_name]].yj = msg_pose_gazebo.orientation().y();
            this->camera_pose_map[this->droneIds[drone_name]].zk = msg_pose_gazebo.orientation().z();

        }

    }

    // now send the packets and increment counter
    for (const auto& pair: this->droneIds){
        // this is the droneID
        PoseTransfer::PoseMessage pose_message{
                    .message_counter = (uint64_t)count,
                    .drone = this->drone_pose_map[pair.second],
                    .camera = this->camera_pose_map[pair.second],
                    .drone_id = pair.second};
        this->poseSender->send_pose_message(pose_message);
        if (count % MESSAGE_THROTTLE == 0) {
            std::cout << "Packet Number: " << count << ", Drone Id: "
                    << std::to_string(pair.second) << ", Timestamp: "
                    << this->getCurrentTimeInFormat() << " , Drone Position: "
                    << std::to_string(this->drone_pose_map[pair.second].x) << ", "
                    << std::to_string(this->drone_pose_map[pair.second].y) << ", "
                    << std::to_string(this->drone_pose_map[pair.second].z) << ", Drone Orientation: "
                    << std::to_string(this->drone_pose_map[pair.second].w) << ", "
                    << std::to_string(this->drone_pose_map[pair.second].xi) << ", "
                    << std::to_string(this->drone_pose_map[pair.second].yj) << ", "
                    << std::to_string(this->drone_pose_map[pair.second].zk)
                    << std::endl;
        }

        this->resetPoseToDefault(pair.second);
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