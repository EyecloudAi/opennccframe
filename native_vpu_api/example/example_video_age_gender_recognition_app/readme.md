## Introduction

This example is used to run synchronous age and gender recognition inference on NCC with notebook UVC camera.

```
image(BGR) -> sync inference -> display
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
$ sudo ./example_video_age_gender_recognition_app  0 (node index, means /dev/video0 )
```
