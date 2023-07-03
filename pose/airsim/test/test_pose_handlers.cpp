#include <gtest/gtest.h>

#include "pose.hpp"
#include "pose_handlers.hpp"
#include "test_vehicle_pose.hpp"

using ::testing::DoAll;
using ::testing::Eq;
using ::testing::SaveArg;


TEST(TestSetDronePose, TestSetDronePose) {
    MockVehiclePose mock_vehicle_interface;

    PoseTransfer::Pose mock_drone_0_pose;
    memset(&mock_drone_0_pose, 0, sizeof(mock_drone_0_pose));
    mock_drone_0_pose.x = 4;
    PoseTransfer::Pose mock_camera_0_pose;
    memset(&mock_camera_0_pose, 0, sizeof(mock_camera_0_pose));
    PoseTransfer::PoseMessage mock_pose_message {
        .message_counter = 3,
        .drone = mock_drone_0_pose,
        .camera = mock_camera_0_pose
    };

    std::mutex mock_mutex_pose_message;

    // exit immediately after one pass for test
    bool exit_flag = 0;

    PoseTransfer::Pose actual_drone_pose_call;
    std::string actual_vehicle_id_call;

    EXPECT_CALL(mock_vehicle_interface, set_vehicle_pose).WillOnce(
        DoAll(
            SaveArg<0>(&actual_drone_pose_call),
            SaveArg<1>(&actual_vehicle_id_call)
        )
    );

    PoseHandlers::set_drone_pose(
        &mock_vehicle_interface,
        &mock_pose_message,
        &mock_mutex_pose_message,
        &exit_flag
    );
    
    EXPECT_EQ(actual_drone_pose_call.x, 4);
    EXPECT_EQ(actual_vehicle_id_call, "");
}