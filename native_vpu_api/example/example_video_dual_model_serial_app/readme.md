## Introduction

This example is used to run serial synchronous face detection and face feature point marking inference with camera video(YU12) streaming on NCC .

```
video(YU12) -> sync inference -> face image(BGR) -> sync inference -> composite display
```



## Prerequisites

This example is based on libusb and opencv. If you have installed, you can omit this step.

```shell
$ sudo apt-get install libusb-dev  libusb-1.0-0-dev
$ sudo apt-get install libopencv-dev
```



## Build

```shell
$ make all
$ cd build
$ sudo ./example_video_dual_model_serial_app /dev/video*
```
