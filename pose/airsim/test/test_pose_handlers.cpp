#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "pose.hpp"
#include "pose_handlers.hpp"
#include "test_vehicle_pose.hpp"

using ::testing::DoAll;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::SaveArg;


TEST(TestSetDronePose, MultiDrone) {
    MockVehiclePose mock_vehicle_interface;

    PoseTransfer::Pose mock_drone_0_pose;
    memset(&mock_drone_0_pose, 0, sizeof(mock_drone_0_pose));
    mock_drone_0_pose.x = 4;
    PoseTransfer::Pose mock_camera_0_pose;
    memset(&mock_camera_0_pose, 0, sizeof(mock_camera_0_pose));
    PoseTransfer::PoseMessage mock_pose_message_0_a {
        .message_counter = 3,
        .drone = mock_drone_0_pose,
        .camera = mock_camera_0_pose,
        .drone_id = 0
    };
    PoseTransfer::PoseMessage mock_pose_message_0_b {
        .message_counter = 4,
        .drone = mock_drone_0_pose,
        .camera = mock_camera_0_pose,
        .drone_id = 0
    };

    PoseTransfer::Pose mock_drone_1_pose;
    memset(&mock_drone_1_pose, 0, sizeof(mock_drone_1_pose));
    mock_drone_1_pose.x = 17;
    PoseTransfer::Pose mock_camera_1_pose;
    memset(&mock_camera_1_pose, 0, sizeof(mock_camera_1_pose));
    PoseTransfer::PoseMessage mock_pose_message_1 {
        .message_counter = 5,
        .drone = mock_drone_1_pose,
        .camera = mock_camera_1_pose,
        .drone_id = 1
    };

    std::mutex mock_mutex_pose_message;

    // exit immediately after one pass for test
    bool exit_flag = 0;

    PoseTransfer::Pose actual_drone_pose_call_0_a;
    std::string actual_vehicle_id_call_0_a;
    PoseTransfer::Pose actual_drone_pose_call_0_b;
    std::string actual_vehicle_id_call_0_b;
    PoseTransfer::Pose actual_drone_pose_call_1;
    std::string actual_vehicle_id_call_1;

    {
        InSequence s;

        EXPECT_CALL(mock_vehicle_interface, set_vehicle_pose).WillOnce(
            DoAll(
                SaveArg<0>(&actual_drone_pose_call_0_a),
                SaveArg<1>(&actual_vehicle_id_call_0_a)
            )
        );
        EXPECT_CALL(mock_vehicle_interface, set_vehicle_pose).WillOnce(
            DoAll(
                SaveArg<0>(&actual_drone_pose_call_0_b),
                SaveArg<1>(&actual_vehicle_id_call_0_b)
            )
        );
        EXPECT_CALL(mock_vehicle_interface, set_vehicle_pose).WillOnce(
            DoAll(
                SaveArg<0>(&actual_drone_pose_call_1),
                SaveArg<1>(&actual_vehicle_id_call_1)
            )
        );
    }

    std::string actual_spawn_drone_id_0;
    std::string actual_spawn_drone_id_1;

    {
        InSequence s;

        EXPECT_CALL(mock_vehicle_interface, spawn_vehicle).WillOnce(
            DoAll(
                SaveArg<0>(&actual_spawn_drone_id_0)
            )
        );
        EXPECT_CALL(mock_vehicle_interface, spawn_vehicle).WillOnce(
            DoAll(
                SaveArg<0>(&actual_spawn_drone_id_1)
            )
        );
    }


    // call the function three independent times with exit_flag = 0 to mimic three loops
    PoseHandlers::set_drone_pose(
        &mock_vehicle_interface,
        &mock_pose_message_0_a,
        &mock_mutex_pose_message,
        &exit_flag
    );
    PoseHandlers::set_drone_pose(
        &mock_vehicle_interface,
        &mock_pose_message_0_b,
        &mock_mutex_pose_message,
        &exit_flag
    );
    PoseHandlers::set_drone_pose(
        &mock_vehicle_interface,
        &mock_pose_message_1,
        &mock_mutex_pose_message,
        &exit_flag
    );
    
    EXPECT_EQ(actual_drone_pose_call_0_a.x, 4);
    EXPECT_EQ(actual_vehicle_id_call_0_a, "0");
    EXPECT_EQ(actual_drone_pose_call_0_b.x, 4);
    EXPECT_EQ(actual_vehicle_id_call_0_b, "0");
    EXPECT_EQ(actual_drone_pose_call_1.x, 17);
    EXPECT_EQ(actual_vehicle_id_call_1, "1");

    EXPECT_EQ(actual_spawn_drone_id_0, "0");
    EXPECT_EQ(actual_spawn_drone_id_1, "1");
}


TEST(TestSetDronePose, MsgOutOfOrder) {
    PoseTransfer::PoseMessage mock_pose_message_0_a {
        .message_counter = 3,
        .drone_id = 10
    };
    // second message should not be called as older than first message
    PoseTransfer::PoseMessage mock_pose_message_0_b {
        .message_counter = 1,
        .drone_id = 10
    };

    MockVehiclePose mock_vehicle_interface;
    std::mutex mock_mutex_pose_message;
    // exit immediately after one pass for test
    bool exit_flag = 1;

    EXPECT_CALL(mock_vehicle_interface, set_vehicle_pose).Times(1);
    EXPECT_CALL(mock_vehicle_interface, spawn_vehicle).Times(1);

    PoseTransfer::PoseMessage *mock_message_pointer = &mock_pose_message_0_a;
    
    // called in separate thread because msg_count must be set each time function is called
    std::thread thread_set_drone_pose(
        PoseHandlers::set_drone_pose,
        &mock_vehicle_interface,
        mock_message_pointer,
        &mock_mutex_pose_message,
        &exit_flag
    );

    // allow time for message_0_a to process
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    mock_mutex_pose_message.lock();
    *mock_message_pointer = mock_pose_message_0_b;
    mock_mutex_pose_message.unlock();
    // allow time for message_0_b to process
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    exit_flag = 0;
    thread_set_drone_pose.join();
}