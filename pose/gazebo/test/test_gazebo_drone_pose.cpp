#include <gtest/gtest.h>

#include <gazebo/msgs/msgs.hh>

#include "gazebo_drone_pose.hpp"
#include "test_udp_sender.hpp"


using ::testing::AtLeast;


/**
 * Each new drone instance in a gazebo pose should get a new id that is sent in the
 * UDPSender pose message
*/
TEST(TestDronePose, TestMultipleDronesSendPoseMessages) {
    gazebo::msgs::PosesStamped mockMsg;
    
    // set drone_0 pose
    gazebo::msgs::Pose* pose0 = mockMsg.add_pose();
    gazebo::msgs::Vector3d position0;
    position0.set_x(1.1);
    position0.set_y(2.2);
    position0.set_z(3.3);

    pose0->set_allocated_position(&position0);

    gazebo::msgs::Quaternion quaternion0;
    quaternion0.set_w(0.7073883);
    quaternion0.set_x(0.0);
    quaternion0.set_y(0.7068252);
    quaternion0.set_z(0.0);

    pose0->set_allocated_orientation(&quaternion0);

    pose0->set_name("drone_0");

    // set drone_1 pose
    gazebo::msgs::Pose* pose1 = mockMsg.add_pose();;
    gazebo::msgs::Vector3d position1;
    position1.set_x(11.11);
    position1.set_y(22.22);
    position1.set_z(33.33);

    pose1->set_allocated_position(&position1);

    gazebo::msgs::Quaternion quaternion1;
    quaternion1.set_w(0.0007963);
    quaternion1.set_x(0.9999997);
    quaternion1.set_y(0.0);
    quaternion1.set_z(0.0);

    pose1->set_allocated_orientation(&quaternion1);

    pose1->set_name("drone_1");

    // somewhere in test above ^
    // test_gazebo_drone_pose(2712,0x202374600) malloc: *** error for object 0x308454128: pointer being freed was not allocated
    // test_gazebo_drone_pose(2712,0x202374600) malloc: *** set a breakpoint in malloc_error_break to debug
    // lldb: https://lldb.llvm.org/use/tutorial.html

    MockUDPSender mockUdpSender;
    GenerateCbLocalPose generateCbLocalPose (&mockUdpSender);
    // https://www.boost.org/doc/libs/1_36_0/libs/smart_ptr/shared_ptr.htm constructor information
    ConstPosesStampedPtr mockConstMsg (&mockMsg);
    generateCbLocalPose.cbLocalPose(mockConstMsg);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}