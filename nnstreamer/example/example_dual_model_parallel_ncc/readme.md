##  Pipeline 

```
compositor ── videoconvert ── xvimagesink
compositor2 ── videoconvert ── xvimagesink

v4l2src ── videorate ── videoconvert ── videoscale ──  tee 

tee ── compositor.sink_0
tee ── videoconvert ── videoscale ── tensor_converter ── tensor_filter ── tensor_decoder ── compositor.sink_1 

tee  ──  compositor2.sink_0
tee  ── videoconvert ── videoscale ── tensor_converter ── tensor_filter ── tensor_decoder ── compositor2.sink_1
```



## Prerequisites

This example is based on libusb and opencv. If you have installed, you can omit this step.

```shell
$ sudo apt-get install libusb-dev  libusb-1.0-0-dev
$ sudo apt-get install libopencv-dev
```



## How to Run

```shell
# Adding environment variables

## For X86 ubuntu system:  
$ sudo -s
$ export GST_PLUGIN_PATH=/usr/local/lib/x86_64-linux-gnu/gstreamer-1.0/

## For ARM-V8 system would be,it is depended on your {libdir}:  
$ sudo -s
$ export GST_PLUGIN_PATH=/usr/local/lib/aarch64-linux-gnu/gstreamer-1.0/

## For RasPi4 (ARMv7l) system would be:  
$ sudo -s
$ export GST_PLUGIN_PATH=/usr/local/lib/arm-linux-gnueabihf/gstreamer-1.0/
it is depended on your {libdir}  

# run application

## CLI
$ gst-launch-1.0 -v compositor name=mix sink_0::zorder=1 sink_1::zorder=2 ! videoconvert ! xvimagesink compositor name=mix2 sink_0::zorder=1 sink_1::zorder=2 ! videoconvert ! xvimagesink v4l2src device=/dev/video2 ! videorate ! videoconvert ! videoscale !  video/x-raw,width=1280,height=720,format=YV12,framerate=15/1 ! tee name=t       t. ! queue ! mix.sink_0       t. ! queue  ! videoconvert ! videoscale ! video/x-raw,width=300,height=300, format=BGR !    tensor_converter ! tensor_transform mode=typecast option=uint8 ! tensor_transform mode=dimchg option=0:2 !         tensor_filter framework=ncc  model=/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/face-detection-retail-0004.blob custom=/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/config/input_BGR.json silent=false accelerator=true  !    tensor_decoder mode=bounding_boxes option1=ov-person-detection option4=1280:720 option5=300:300 ! mix.sink_1 t. ! queue ! mix2.sink_0 t. ! queue  ! videoconvert ! videoscale ! video/x-raw,width=544,height=320,format=BGR !     tensor_converter ! tensor_transform mode=typecast option=uint8 ! tensor_transform mode=dimchg option=0:2 !         tensor_filter framework=ncc  model=/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/person-detection-retail-0013/person-detection-retail-0013.blob custom=/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/person-detection-retail-0013/config/input_BGR.json silent=false accelerator=true  !  tensor_decoder mode=bounding_boxes option1=ov-person-detection option4=1280:720 option5=544:320 ! mix2.sink_2

## Python
$ python3 example_dual_model_parallel_ncc.py /dev/video*
```



