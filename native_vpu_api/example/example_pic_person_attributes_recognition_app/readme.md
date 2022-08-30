## Introduction

This example is used to run synchronous person attributes recognition inference on NCC with image.
Open Config, change the pedestrian characteristics you want to filter to 1, and run the program.
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
$ sudo ./example_pic_person_attributes_recognition_app ../test.jpeg
```
