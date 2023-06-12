#include <gazebo/msgs/msgs.hh>

#include "send_drone_pose_pi.hpp"


/**
 * Each new drone instance in a gazebo pose should get a new id that is sent in the
 * UDPSender pose message
*/
void testCbLocalPoseDroneIds() {
    gazebo::msgs::PosesStamped mockMsg;
    
    // set drone_0 pose
    gazebo::msgs::Pose pose0;
    gazebo::msgs::Vector3d position0;
    position0.set_x(1.1);
    position0.set_y(2.2);
    position0.set_z(3.3);

    pose0.set_allocated_position(position0);

    gazebo::msgs::Quaternion quaternion0;
    quaternion0.set_w(0.7073883);
    quaternion0.set_x(0.0);
    quaternion0.set_y(0.7068252);
    quaternion0.set_z(0.0);

    pose0.set_allocated_position(quaternion0);

    pose0.set_name("drone_0");

    mockMsg.add_pose(pose0);

    // set drone_1 pose
    gazebo::msgs::Pose pose1;
    gazebo::msgs::Vector3d position1;
    position1.set_x(11.11);
    position1.set_y(22.22);
    position1.set_z(33.33);

    pose1.set_allocated_position(position1);

    gazebo::msgs::Quaternion quaternion1;
    quaternion1.set_w(0.0007963);
    quaternion1.set_x(0.9999997);
    quaternion1.set_y(0.0);
    quaternion1.set_z(0.0);

    pose0.set_allocated_position(quaternion1);

    pose1.set_name("drone_1");

    mockMsg.add_pose(pose1);
}