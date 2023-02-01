# Gstreamer-Airsim
This repository includes gstreamer pipelines to stream video frames from Unreal Engine based [AirSim's camera's](https://microsoft.github.io/AirSim/image_apis/) over UDP.

## Dependencies
- [Airsim](https://microsoft.github.io/AirSim/) ([+Unreal Engine](https://www.unrealengine.com/en-US/download))
- [Gstreamer](https://gstreamer.freedesktop.org/documentation/installing/index.html?gi-language=c)
- gcc / g++ compiler

## Building
Note: when building on Mac, [gstreamer recommends](https://gstreamer.freedesktop.org/documentation/installing/on-mac-osx.html#manual-compilation-with-pkgconfig) the following environment variable updates are made:
```bash
# Tell pkg-config where to find the .pc files
$ export PKG_CONFIG_PATH=/Library/Frameworks/GStreamer.framework/Versions/1.0/lib/pkgconfig

# We will use the pkg-config provided by the GStreamer.framework
$ export PATH=/Library/Frameworks/GStreamer.framework/Versions/1.0/bin:$PATH
```
Depending on your installation, the paths may be different from above. For example, if installed with `brew` then the path may be `/opt/homebrew/Cellar/gstreamer/...` With `brew`, the gstreamer packages including various elements (base, good, bad, ugly) will be installed separately as well.

### Airsim send
Note: Only has been built using an ARM Mac to date. It may work on other Linux devices, but isn't guarunteed to. 

_Additional dependencies:_
- cmake

Set required environment variables:
- AIRSIM_ROOT - AirSim install directory (ie. `/Users/<user>/repos/AirSim`)
- GSTREAMER_ROOT - Gstreamer install directory (ie. `/Library/Frameworks/GStreamer.framework` - this will be different if installed via a package manager or in a linux environment other than Mac)

Create a build directory and run cmake / make:
```bash
# starting from 'send' dir
mkdir build
cd build
cmake ..
make
```

### Jetson receive
Used to test that video is being successfully streamed from airsim to a Nvidia Jetson device. Must be build on the Jetson as it uses Nvidia specific gstreamer libraries to interace with the Nvidia GPU. 

```bash
# starting from the 'receive/jetson' dir
mkdir build
g++ ./src/udp-cam-receive-jetson.cpp -o ./build/udp-cam-receive-jetson `pkg-config --cflags --libs gstreamer-1.0`
```

### Local receive
Note: Only has been built using an ARM Mac to date. It may work on other Linux devices, but isn't guarunteed to. 

```bash
# starting from 'receive/mac'
mkdir build
g++ ./src/udp-cam-receive.cpp -o ./build/udp-cam-receive `pkg-config --cflags --libs gstreamer-1.0`
```

## Running
### Airsim send
After compiling from the build dir:
```bash
./AirSimCamSend -a <dest ip addr> -p <port> -f <desired frame rate>
```

All inputs are optional and will default to 'localhost', 5000, and 15 fps respectively. 

### Jetson receive
After compiling from the build dir:
```bash
./udp-cam-receive-jetson -p <port>
```

Port input is optional and will default to 5000.

Video window should open and display at the resolution and frame rate that the sender is streaming.

### Local receive
After compiling from the build dir:
```bash
./udp-cam-receive -p <port>
```

Port input is optional and will default to 5000.

Video window should open and display at the resolution and frame rate that the sender is streaming.


