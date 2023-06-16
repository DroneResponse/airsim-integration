#include <gtest/gtest.h>

#include <gazebo/msgs/msgs.hh>

#include "gazebo_drone_pose.hpp"
#include "test_udp_sender.hpp"

https://google.github.io/googletest/gmock_for_dummies.html#using-mocks-in-tests
using ::testing::AtLeast;


/**
 * Each new drone instance in a gazebo pose should get a new id that is sent in the
 * UDPSender pose message
*/
TEST(TestDronePose, TestMultipleDronesSendPoseMessages) {
    gazebo::msgs::PosesStamped mockMsg;
    
    // set drone_0 pose
    gazebo::msgs::Pose* pose0 = mockMsg.add_pose();
    gazebo::msgs::Vector3d* position0 = pose0->mutable_position();
    position0->set_x(1.1);
    position0->set_y(2.2);
    position0->set_z(3.3);

    gazebo::msgs::Quaternion* quaternion0 = pose0->mutable_orientation();
    quaternion0->set_w(0.7073883);
    quaternion0->set_x(0.0);
    quaternion0->set_y(0.7068252);
    quaternion0->set_z(0.0);

    pose0->set_name("drone_0");

    // set drone_1 pose
    gazebo::msgs::Pose* pose1 = mockMsg.add_pose();;
    gazebo::msgs::Vector3d* position1 = pose1->mutable_position();
    position1->set_x(11.11);
    position1->set_y(22.22);
    position1->set_z(33.33);

    gazebo::msgs::Quaternion* quaternion1 = pose1->mutable_orientation();
    quaternion1->set_w(0.0007963);
    quaternion1->set_x(0.9999997);
    quaternion1->set_y(0.0);
    quaternion1->set_z(0.0);

    pose1->set_name("drone_1");

    MockUDPSender mockUdpSender;
    EXPECT_CALL(mockUdpSender, create_socket()).Times(1);

    GenerateCbLocalPose generateCbLocalPose (&mockUdpSender);

    https://www.boost.org/doc/libs/1_55_0/libs/smart_ptr/shared_ptr.htm
    ConstPosesStampedPtr mockConstMsgPtr ( new const gazebo::msgs::PosesStamped(mockMsg) );
    generateCbLocalPose.cbLocalPose(mockConstMsgPtr);
    
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); // https://google.github.io/googletest/primer.html
    return RUN_ALL_TESTS();
}