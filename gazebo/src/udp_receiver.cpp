#include "udp_receiver.hpp"


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