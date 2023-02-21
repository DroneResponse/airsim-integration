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
export PKG_CONFIG_PATH=/Library/Frameworks/GStreamer.framework/Versions/1.0/lib/pkgconfig

# We will use the pkg-config provided by the GStreamer.framework
export PATH=/Library/Frameworks/GStreamer.framework/Versions/1.0/bin:$PATH
```
Depending on your installation, the paths may be different from above. For example, if installed with `brew` then the path may be `/opt/homebrew/Cellar/gstreamer/...` With `brew`, the gstreamer packages including various elements (base, good, bad, ugly) will be installed separately as well.

You can alternately [compile directly](https://gstreamer.freedesktop.org/documentation/installing/on-mac-osx.html#manual-compilation) with flags to headers, libraries and frameworks.

When building on Linux, you should be able to use a pre-existing installation of pkg-config and do not need to set the env variables above.

### Airsim send
_Additional dependencies:_
- cmake

Set required environment variable:
- AIRSIM_ROOT - AirSim install directory (ie. `/Users/<user>/repos/AirSim`)

If you are compiling on mac and gstreamer is installed as a /Library/Framework, you will want to set the following environment variable so the required headers are successfully found
- GSTREAMER_ROOT - Gstreamer install directory (ie. `/Library/Frameworks/GStreamer.framework`)

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
# starting from 'receive/linux'
mkdir build
g++ ./src/udp-cam-receive.cpp -o ./build/udp-cam-receive `pkg-config --cflags --libs gstreamer-1.0`
```

### Pose
_Additional dependencies:_
- cmake
- [px4 gazebo](https://docs.px4.io/main/en/simulation/gazebo.html)
    - for Arm Mac's, must be [compiled using rosetta in X86 terminal](https://docs.px4.io/main/en/dev_setup/dev_env_mac.html#macos-development-environment)
    - the same X86 terminal will need to be used to compile gazebo send_drone_pose in this repo

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
./airsim-cam-send -a <dest ip addr> -p <port> -f <desired frame rate>
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

### Pose
The gazebo pose sender and airsim pose receiver are two separate programs that communicate over UDP. This allows them to be run on separate machines or seperate containers. 

- An Airsim simulation must be started before starting the pose receiver so that the receiver successfully connects to the simulation's API
- If utilizing camera pose, make sure to use the gazebo `typhoon_h480` drone that includes a camera on a gimbal and publishes the camera pose

After compiling from the gazebo build dir:
```bash
./send_drone_pose -p <port> -a <address>
```

Port and address are optional and will default to `50000` and `127.0.0.1` respectively.

After compiling from the airsim build dir:
```bash
./receive_drone_pose -p <port> -c -r
```

Port is optional and will default to `50000`

The optional `-c` flag indicates that the Airsim drone pose should be set to the gazebo typhoon_h480 gimbal camera pose. This will pose the Airsim drone in line with the camera pose in gazebo. Doing so essentially treats the Airsim drone as the gimbal camera.

The optional `-r` flag will artificially override and remove any gimbal camera roll.

No flag will set the Airsim drone pose to the gazebo drone's pose.


