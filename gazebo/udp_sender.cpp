#include "UDPSender.h"
#include <sys/socket.h>
#include <iostream>
#include <string>

class UDPSender {       // The class
    public:             // Access specifier
        UDPSender(string host, uint_t16 dest_port);  // Constructor
        void send_pose_message(const PoseMessage& pose_message);  // Method/function declaration
    private:
        uint_t16 dest_port;
        string host;  // Attribute (string variable)
        socket_t socket;
};

UDPSender::UDPSender(string host, uint_t16 dest_port) {
    this->dest_port = dest_port;
    this->host = host;

    struct hostent *server = gethostbyname(this->host.c_str());

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    struct sockaddr_in dest_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(dest_port),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };
    if (!bind(sock, (struct sockaddr *)&addr, sizeof(addr))) {
        std::cout << "bind failed" << std::endl;
        exit(1);
    };
    socket = socket_t(air_sim_host, air_sim_port);
}

void UDPSender::send_pose_message(const PoseMessage& pose_message) {
    // send pose message to airsim
    send(socket, &pose_message, sizeof(PoseMessage));
}