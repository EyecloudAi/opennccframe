## Introduction

This example is used to introduce basic application programming interface of camera controller .



## Prerequisites

This example is based on libusb and opencv. If you have installed, you can omit this step.

```shell
$ sudo apt-get install libusb-dev  libusb-1.0-0-dev
$ sudo apt-get install libopencv-dev
```



## Build

```shell
make all
cd build
sudo ./example_video_control_app /dev/video*
```

