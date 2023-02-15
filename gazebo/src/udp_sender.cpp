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


UDPSender::~UDPSender() {
    if (!shutdown(this->sock, SHUT_RDWR)) {
        std::cerr << "udp sender socket not successfully closed" << std::endl;
    };

    std::cout << "udp sender socket successfully closed" << std::endl;
}


void UDPSender::send_pose_message(const PoseTransfer::PoseMessage pose_message) {
    // send pose message to airsim
    PoseTransfer::UdpPoseMessage udp_pose_message = this->pose_to_udp_message(pose_message);
    send(this->sock, &udp_pose_message, sizeof(PoseTransfer::PoseMessage), 0);
}


PoseTransfer::UdpPoseMessage UDPSender::pose_to_udp_message(
    const PoseTransfer::PoseMessage pose_message) {
    PoseTransfer::UdpPose drone_udp_pose {
        .x = htonll(this->double_to_udp_uint64(pose_message.drone.x)),
        .y = htonll(this->double_to_udp_uint64(pose_message.drone.y)),
        .z = htonll(this->double_to_udp_uint64(pose_message.drone.z)),
        .w = htonll(this->double_to_udp_uint64(pose_message.drone.w)),
        .xi = htonll(this->double_to_udp_uint64(pose_message.drone.xi)),
        .yj = htonll(this->double_to_udp_uint64(pose_message.drone.yj)),
        .zk = htonll(this->double_to_udp_uint64(pose_message.drone.zk))
    };
    PoseTransfer::UdpPose camera_udp_pose {
        .x = htonll(this->double_to_udp_uint64(pose_message.camera.x)),
        .y = htonll(this->double_to_udp_uint64(pose_message.camera.y)),
        .z = htonll(this->double_to_udp_uint64(pose_message.camera.z)),
        .w = htonll(this->double_to_udp_uint64(pose_message.camera.w)),
        .xi = htonll(this->double_to_udp_uint64(pose_message.camera.xi)),
        .yj = htonll(this->double_to_udp_uint64(pose_message.camera.yj)),
        .zk = htonll(this->double_to_udp_uint64(pose_message.camera.zk))
    };

    return PoseTransfer::UdpPoseMessage {
        .message_counter = htonll(pose_message.message_counter),
        .drone = drone_udp_pose,
        .camera = camera_udp_pose
    };
}


uint64_t UDPSender::double_to_udp_uint64(double d_field) {
    return (uint64_t) (d_field * PoseTransfer::udp_decimal_offset);
}