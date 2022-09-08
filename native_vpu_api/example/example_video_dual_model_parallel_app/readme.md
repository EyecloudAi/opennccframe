## Introduction

This example is used to run dual asynchronous inference on NCC.

- run face detection with video streaming
- run person detection with image

```
video(YV12) -> async inference -> display
image(BGR) -> async inference -> display
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
$ sudo ./example_video_dual_model_parallel_app /dev/openncc ../../pic_lib/person.jpg
```

