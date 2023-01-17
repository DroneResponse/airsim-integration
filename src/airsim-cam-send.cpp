// compile only working: g++ -std=c++0x -c ./src/airsim-cam-send.cpp -o airsim-cam-send -I/Users/jasonbrauer/drone_response/AirSim/AirLib/include -I/Users/jasonbrauer/drone_response/AirSim/AirLib/deps/eigen3
// w/ linking failing: g++ -std=c++0x ./src/airsim-cam-send.cpp -o airsim-cam-send -I/Users/jasonbrauer/drone_response/AirSim/AirLib/include -I/Users/jasonbrauer/drone_response/AirSim/AirLib/deps/eigen3 -L/Users/jasonbrauer/drone_response/AirSim/build_release/output/lib

#include "common/common_utils/StrictMode.hpp"
STRICT_MODE_OFF
#ifndef RPCLIB_MSGPACK
#define RPCLIB_MSGPACK clmdep_msgpack
#endif // !RPCLIB_MSGPACK
// #include "rpc/rpc_error.h"
STRICT_MODE_ON

#include "vehicles/multirotor/api/MultirotorRpcLibClient.hpp"
#include <iostream>
#include <chrono>
#include <thread>

using namespace msr::airlib;
MultirotorRpcLibClient client;

static vector<uint8_t> getOneImage() {
    // getImages provides more details about the image
    return client.simGetImage("front_center", ImageCaptureBase::ImageType::Scene);
}

static void sendImageStream(int fps) {
    printf("Milliseconds between frames: %d\n", (int)((1 / (float) fps) * 1e3));
    while(1) {
        std::cout << "Image unit8 size: " << getOneImage().size() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds((int)((1 / (float) fps) * 1e3)));
    }
}

int main() {
    sendImageStream(30);
}

