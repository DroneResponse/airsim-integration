#include "gmock/gmock.h"

#include "pose.hpp"
#include "udp_sender.hpp"

class MockUDPSender : public UDPSender {
    public:
        MockUDPSender() : UDPSender("1.1.1.1", 5555) {};
        MOCK_METHOD(void, send_pose_message, (PoseTransfer::PoseMessage pose_message));
};