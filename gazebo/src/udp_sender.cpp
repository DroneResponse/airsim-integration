#include "udp_sender.hpp"


UDPSender::UDPSender(std::string host, unsigned short dest_port) {
    this->dest_port = dest_port;
    this->host = host;

    struct addrinfo hints;
    struct addrinfo *result, *rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0; // ignored when value set for node
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    // https://man7.org/linux/man-pages/man3/getaddrinfo.3.html
    int response_code = getaddrinfo(
        this->host.c_str(),
        std::to_string(this->dest_port).c_str(),
        &hints,
        &result);

    if (response_code != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(response_code) << std::endl;
        exit(-1);
    }

    // loop through all addrinfo objects returned by getaddrinfo in case there are multiple
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        this->sock = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (this->sock == -1)
            continue;

        if (connect(this->sock, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        // shut down and sockets that don't connect
        if (!shutdown(this->sock, SHUT_RDWR)) {
            std::cerr << "udp sender socket not successfully closed" << std::endl;
        };
    }

    freeaddrinfo(result);

    if (rp == NULL) {
        std::cerr << "Connection could not be established" << std::endl;
        exit(-1);
    }
}


void UDPSender::send_pose_message(const PoseTransfer::PoseMessage pose_message) {
    // send pose message to airsim
    send(this->sock, &this->pose_to_udp_message(pose_message), sizeof(PoseTransfer::PoseMessage), 0);
}


UDPSender::~UDPSender() {
    if (!shutdown(this->sock, SHUT_RDWR)) {
        std::cerr << "udp sender socket not successfully closed" << std::endl;
    };

    std::cout << "udp sender socket successfully closed" << std::endl;
}


PoseTransfer::UdpPoseMessage UDPSender::pose_to_udp_message(
    const PoseTransfer::PoseMessage pose_message) {
    PoseTransfer::UdpPose drone_udp_pose {
        .x = htonll(pose_message.drone.x * PoseTransfer::udp_decimal_offset),
        .y = htonll(pose_message.drone.y * PoseTransfer::udp_decimal_offset),
        .z = htonll(pose_message.drone.z * PoseTransfer::udp_decimal_offset),
        .w = htonll(pose_message.drone.w * PoseTransfer::udp_decimal_offset),
        .xi = htonll(pose_message.drone.xi * PoseTransfer::udp_decimal_offset),
        .yj = htonll(pose_message.drone.yj * PoseTransfer::udp_decimal_offset),
        .zk = htonll(pose_message.drone.zk * PoseTransfer::udp_decimal_offset)
    };
    PoseTransfer::UdpPose camera_udp_pose {
        .x = htonll(pose_message.camera.x * PoseTransfer::udp_decimal_offset),
        .y = htonll(pose_message.camera.y * PoseTransfer::udp_decimal_offset),
        .z = htonll(pose_message.camera.z * PoseTransfer::udp_decimal_offset),
        .w = htonll(pose_message.camera.w * PoseTransfer::udp_decimal_offset),
        .xi = htonll(pose_message.camera.xi * PoseTransfer::udp_decimal_offset),
        .yj = htonll(pose_message.camera.yj * PoseTransfer::udp_decimal_offset),
        .zk = htonll(pose_message.camera.zk * PoseTransfer::udp_decimal_offset)
    };

    return PoseTransfer::UdpPoseMessage {
        .message_counter = htonll(pose_message.message_counter),
        .drone = drone_udp_pose,
        .camera = camera_udp_pose
    };
}