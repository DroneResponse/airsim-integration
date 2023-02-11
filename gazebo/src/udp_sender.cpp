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

    // struct hostent *server = gethostbyname(this->host.c_str());
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

void UDPSender::send_pose_message(const PoseMessage& pose_message) {
    // send pose message to airsim
    send(this->sock, &pose_message, sizeof(PoseMessage));
}


UDPSender::~UDPSender() {
    if (!shutdown(this->sock, SHUT_RDWR)) {
        std::cerr << "udp sender socket not successfully closed" << std::endl;
    };

    std::cout << "udp sender socket successfully closed" << std::endl;
}