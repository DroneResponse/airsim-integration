# Airsim-Integration
Provides programs to stream drone and camera poses from Gazebo to AirSim over UDP. Also includes gstreamer programs to stream video frames from Unreal Engine based [AirSim camera's](https://microsoft.github.io/AirSim/image_apis/) over UDP. Includes UDP camera stream receivers built both for general linux based machines and Nvidia Jetsons specifically. 

## Dependencies
- [Airsim](https://microsoft.github.io/AirSim/) ([+Unreal Engine](https://www.unrealengine.com/en-US/download))
- [Gstreamer](https://gstreamer.freedesktop.org/documentation/installing/index.html?gi-language=c)
- gcc / g++ compiler

## Building
Note: when building on Mac via package configs, [gstreamer recommends](https://gstreamer.freedesktop.org/documentation/installing/on-mac-osx.html#manual-compilation-with-pkgconfig) the following environment variable updates are made:
```bash
# Tell pkg-config where to find the .pc files
$ export PKG_CONFIG_PATH=/Library/Frameworks/GStreamer.framework/Versions/1.0/lib/pkgconfig

# We will use the pkg-config provided by the GStreamer.framework
$ export PATH=/Library/Frameworks/GStreamer.framework/Versions/1.0/bin:$PATH
```
Depending on your installation, the paths may be different from above. For example, if installed with `brew` then the path may be `/opt/homebrew/Cellar/gstreamer/...` With `brew`, the gstreamer packages including various elements (base, good, bad, ugly) will be installed separately as well.

You can alternately [compile directly](https://gstreamer.freedesktop.org/documentation/installing/on-mac-osx.html#manual-compilation) with flags to headers, libraries and frameworks. 

### Airsim send
Note: Only has been built using an Arm Mac to date. It may work on other Linux devices, but isn't guarunteed without modifications. 

_Additional dependencies:_
- cmake

Set required environment variables:
- AIRSIM_ROOT - AirSim install directory (ie. `/Users/<user>/repos/AirSim`)
- GSTREAMER_ROOT - Gstreamer install directory (ie. `/Library/Frameworks/GStreamer.framework` - this will be different if installed via a package manager or in a linux OS other than Mac)

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
Note: Only has been built using an Arm Mac to date. It may work on other Linux devices, but isn't guarunteed without modifications. 

```bash
# starting from 'receive/mac'
mkdir build
g++ ./src/udp-cam-receive.cpp -o ./build/udp-cam-receive `pkg-config --cflags --libs gstreamer-1.0`
```

### Pose
_Additional dependencies:_
- cmake
- [px4 gazebo](https://docs.px4.io/main/en/simulation/gazebo.html)
    - for Arm Mac's, must be [compiled using rosetta in X86 terminal](https://docs.px4.io/main/en/dev_setup/dev_env_mac.html#macos-development-environment)
    - the same X86 terminal will need to be used to compile gazebo DronePose in this repo

Set required environment variables:
- AIRSIM_ROOT - AirSim install directory (ie. `/Users/<user>/repos/AirSim`)

```bash
# starting from 'gazebo' dir
mkdir build
cd build
cmake -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ..
make
```


## Running
### Airsim send - sends airsim stream from front-center drone camera
After compiling from the build dir:
```bash
./AirSimCamSend -a <dest ip addr> -p <port> -f <desired frame rate>
```

All inputs are optional and will default to 'localhost', 5000, and 15 fps respectively. 

### Jetson receive - receives stream on jetson
After compiling from the build dir:
```bash
./udp-cam-receive-jetson -p <port>
```

Port input is optional and will default to 5000.

Video window should open and display at the resolution and frame rate that the sender is streaming.

### Local receive - receives stream on Mac (linux)
After compiling from the build dir:
```bash
./udp-cam-receive -p <port>
```

Port input is optional and will default to 5000.

Video window should open and display at the resolution and frame rate that the sender is streaming.

### Gazebo
- Gazebo and AirSim simulations need to be running for this program to translate gazebo poses to AirSim poses
    - the gazebo simulation can be headless
- If utilizing camera pose, make sure to use the gazebo `typhoon_h480` drone that includes a camera on a gimbal and publishes the camera pose

After compiling from the build dir:
```bash
./DronePose <optional -c>
```

The `-c` flag indicates that the AirSim drone pose should be set to the gazebo typhoon_h480 gimbal camera pose. This will pose the camera in line with the camera pose in gazebo.

The `-r` flag will artificially override and remove any gimbal camera roll.

No flag will set the drone pose to the gazebo drone's pose.


