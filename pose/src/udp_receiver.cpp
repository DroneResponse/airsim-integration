#include "udp_receiver.hpp"
#include "net_endian.hpp"
#include <cstring>

UDPReceiver::UDPReceiver(unsigned short int listener_port) {
    this->listener_port = listener_port;

    struct addrinfo hints;
    struct addrinfo *result, *rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // ignored when value set for node
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    // https://man7.org/linux/man-pages/man3/getaddrinfo.3.html
    int response_code = getaddrinfo(
        NULL,
        std::to_string(this->listener_port).c_str(),
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

        if (bind(this->sock, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        // shut down and sockets that don't connect
        if (!shutdown(this->sock, SHUT_RDWR)) {
            std::cerr << "udp receiver socket not successfully closed" << std::endl;
        };
    }

    freeaddrinfo(result);

    if (rp == NULL) {
        std::cerr << "Bind to port " << std::to_string(listener_port) << " failed" << std::endl;
        exit(-1);
    }
}


UDPReceiver::~UDPReceiver() {
    if (!shutdown(this->sock, SHUT_RDWR)) {
        (std::cerr << "udp receiver socket on port " << std::to_string(this->listener_port) 
        << " not successfully closed" << std::endl);
    };

    std::cout << "udp receiver socket successfully closed" << std::endl;
}


void UDPReceiver::listen_pose_message(
    PoseTransfer::PoseMessage *pose_message,
    std::mutex &mutex_pose_message) {
    PoseTransfer::UdpPoseMessage udp_pose_message;
    while(1) {
        if (!recv(this->sock, (char*) &udp_pose_message, sizeof(udp_pose_message), 0)) {
            std::cout << "listen_pose_message - received empty message";
        }
        mutex_pose_message.lock();
        this->udp_message_to_pose(udp_pose_message, pose_message);
        mutex_pose_message.unlock();
    }
}


void UDPReceiver::udp_message_to_pose(
    PoseTransfer::UdpPoseMessage udp_pose_message,
    PoseTransfer::PoseMessage *pose_message) {
    PoseTransfer::Pose drone_pose {
        .x = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.drone.x)),
        .y = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.drone.y)),
        .z = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.drone.z)),
        .w = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.drone.w)),
        .xi = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.drone.xi)),
        .yj = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.drone.yj)),
        .zk = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.drone.zk))
    };
    PoseTransfer::Pose camera_pose {
        .x = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.camera.x)),
        .y = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.camera.y)),
        .z = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.camera.z)),
        .w = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.camera.w)),
        .xi = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.camera.xi)),
        .yj = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.camera.yj)),
        .zk = this->udp_int64_to_double(net_bits::ntoh64(udp_pose_message.camera.zk))
    };
    
    pose_message->message_counter = net_bits::ntoh64(udp_pose_message.message_counter);
    pose_message->drone = drone_pose;
    pose_message->camera = camera_pose;
}


double UDPReceiver::udp_int64_to_double(int64_t u_field) {
    return (double) u_field / PoseTransfer::udp_decimal_offset;
}