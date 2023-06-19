#include <gtest/gtest.h>

#include <gazebo/msgs/msgs.hh>

#include "gazebo_drone_pose.hpp"
#include "pose.hpp"
#include "test_udp_sender.hpp"

// https://google.github.io/googletest/gmock_for_dummies.html#using-mocks-in-tests
using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::SaveArg;
using ::testing::DoAll;


/**
 * Each new drone instance in a gazebo pose should get a new id that is sent in the
 * UDPSender pose message
*/
TEST(TestDronePose, TestMultipleDronesSendPoseMessages) {
    PoseTransfer::Pose mock_drone_0_pose {
        .x = 1.1,
        .y = 2.2,
        .z = 3.3,
        .w = 0.7073883,
        .xi = 0.0,
        .yj = 0.7068252,
        .zk = 0.0
    };
    PoseTransfer::Pose mock_camera_0_pose;
    memset(&mock_camera_0_pose, 0, sizeof(mock_camera_0_pose));
    PoseTransfer::PoseMessage mock_pose_message_0 {
        .message_counter = 0,
        .drone = mock_drone_0_pose,
        .camera = mock_camera_0_pose
    };
    PoseTransfer::Pose mock_drone_1_pose {
        .x = 11.11,
        .y = 22.22,
        .z = 33.33,
        .w = 0.0007963,
        .xi = 0.9999997,
        .yj = 0.0,
        .zk = 0.0
    };
    PoseTransfer::Pose mock_camera_1_pose;
    memset(&mock_camera_1_pose, 0, sizeof(mock_camera_1_pose));
    PoseTransfer::PoseMessage mock_pose_message_1 {
        .message_counter = 1,
        .drone = mock_drone_1_pose,
        .camera = mock_camera_1_pose
    };
    
    //  need to figure out if each drone sends its own message or all poses come in on one message
    gazebo::msgs::PosesStamped mockMsg;
    
    // set drone_0 pose
    gazebo::msgs::Pose* pose0 = mockMsg.add_pose();
    gazebo::msgs::Vector3d* position0 = pose0->mutable_position();
    position0->set_x(mock_drone_0_pose.x);
    position0->set_y(mock_drone_0_pose.y);
    position0->set_z(mock_drone_0_pose.z);

    gazebo::msgs::Quaternion* quaternion0 = pose0->mutable_orientation();
    quaternion0->set_w(mock_drone_0_pose.w);
    quaternion0->set_x(mock_drone_0_pose.xi);
    quaternion0->set_y(mock_drone_0_pose.yj);
    quaternion0->set_z(mock_drone_0_pose.zk);

    pose0->set_name("drone_0");

    // set drone_0 camera pose
    gazebo::msgs::Pose* pose0_cam0 = mockMsg.add_pose();
    gazebo::msgs::Vector3d* position0_cam0 = pose0_cam0->mutable_position();
    position0_cam0->set_x(mock_camera_0_pose.x);
    position0_cam0->set_y(mock_camera_0_pose.y);
    position0_cam0->set_z(mock_camera_0_pose.z);

    gazebo::msgs::Quaternion* quaternion0_cam0 = pose0_cam0->mutable_orientation();
    quaternion0_cam0->set_w(mock_camera_0_pose.w);
    quaternion0_cam0->set_x(mock_camera_0_pose.xi);
    quaternion0_cam0->set_y(mock_camera_0_pose.yj);
    quaternion0_cam0->set_z(mock_camera_0_pose.zk);

    pose0_cam0->set_name("drone_0::cg03_camera_link");


    // set drone_1 pose
    gazebo::msgs::Pose* pose1 = mockMsg.add_pose();;
    gazebo::msgs::Vector3d* position1 = pose1->mutable_position();
    position1->set_x(mock_drone_1_pose.x);
    position1->set_y(mock_drone_1_pose.y);
    position1->set_z(mock_drone_1_pose.z);

    gazebo::msgs::Quaternion* quaternion1 = pose1->mutable_orientation();
    quaternion1->set_w(mock_drone_1_pose.w);
    quaternion1->set_x(mock_drone_1_pose.xi);
    quaternion1->set_y(mock_drone_1_pose.yj);
    quaternion1->set_z(mock_drone_1_pose.zk);

    pose1->set_name("drone_1");

    MockUDPSender mockUdpSender;
    EXPECT_CALL(mockUdpSender, create_socket()).Times(1);
    PoseTransfer::PoseMessage actual_pose_message;
    // trying to validate send_pose_message called with PoseMessage containing expected values
    // https://google.github.io/googletest/gmock_cook_book.html#SaveArgVerify
    EXPECT_CALL(mockUdpSender, send_pose_message).WillOnce(DoAll(SaveArg<0>(&actual_pose_message)));
    // EXPECT_THAT(actual_pose_message.)

    GenerateCbLocalPose generateCbLocalPose (&mockUdpSender);

    https://www.boost.org/doc/libs/1_55_0/libs/smart_ptr/shared_ptr.htm
    ConstPosesStampedPtr mockConstMsgPtr ( new const gazebo::msgs::PosesStamped(mockMsg) );
    generateCbLocalPose.cbLocalPose(mockConstMsgPtr);
    
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); // https://google.github.io/googletest/primer.html
    return RUN_ALL_TESTS();
}