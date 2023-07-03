#include "gmock/gmock.h"

#include "pose.hpp"
#include "pose_sender.hpp"

#ifndef TEST_POSE_SENDER_H
#define TEST_POSE_SENDER_H

class MockPoseSender : public PoseSender {
    public:
        MockPoseSender() : PoseSender("1.1.1.1", 5555) {};
        MOCK_METHOD(void, send_pose_message, (PoseTransfer::PoseMessage pose_message));
        MOCK_METHOD(void, create_socket, ());
};

#endif