#include "gmock/gmock.h"

#include "pose.hpp"
#include "udp_sender.hpp"

class MockUDPSender : public UDPSender {
    public:
        MOCK_METHOD(void, send_pose_message, (PoseTransfer::PoseMessage));
};