## OpenNCC Native SDK

Openncc native SDK is an upgraded version of [openncc API 1.0](https://github.com/EyecloudAi/openncc). It provides cross platform driver support for openncc USB camera and openncc SOM accelerator card, manages and allocates reasoning engine resources, including but not limited to x86: Ubuntu, rk3568, raspberry PI, NVIDIA.The openncc native SDK has been encapsulated as a sub plug-in of nnstreamer to provide the underlying resource scheduling interface for inference media pipeline. It is a proprietary development package of openncc frame supporting openncc based on Intel ma2480 Vpu hardware products.  

The openncc native SDK provides the following working modes:  

### USB AI Acceleration Card(As Intel NCS) mode

![ncs mode][1]

* In the accelerator card mode, the host app obtains the video stream from local file, IPC, webcam or v4l2 Mipi CAM.Could configure the preprocessing module according to the resolution and the format of the input image of the AI model, sends the images to the openncc SOM through the openncc native SDK, and device returns the AI-meta results. The inference supports asynchronous and synchronous modes. The inference pipeline on the openncc SOM could be configured through the [openncc model JSON](https://eyecloudai.github.io/opennccframe/tutorials/how-to-write-json-config.html).

* Openncc SOM can support 6 inference pipelines configuration locally, and 2 pipelines can run concurrently in real time. Users can realize multi-level chain or multi model concurrency of inference through intermediate processing of host app.

  ![multi Acce Cards ][2]

 * Multiple accelerator cards are also supported. Users can expand the number of accelerator cards according to their computing abilities needs, and the SDK will dynamically allocate computing abilities.

### UVC Web Camera with AI Acceleration Card(As Intel NCS) mode   

![UVC with NCS][3]

* In this mode, openncc is a combination of USB camera and AI accelerator card. The HD and 4K sensors supported by openncc are connected. After completing the ISP on the VPU, it outputs the video stream to the Host APP as a standard UVC camera.
* After the Host APP obtains the video stream, it carries out preprocessing, configures the preprocessing module according to the resolution and format of the input image of the inference model, download the image to the openncc SOM for inference through the openncc native SDK, and returns the AI results. The inference supports asynchronous and synchronous modes. The inference pipeline on the openncc SOM could be configured through the [openncc model JSON](https://eyecloudai.github.io/opennccframe/tutorials/how-to-write-json-config.html).

### UVC AI Camera with AI Acceleration Card(As Intel NCS) mode

![ai cam with ncs mode][4]

* In this mode, openncc is a combination of USB camera and AI accelerator card. The HD and 4K sensors supported by openncc are connected. After completing the ISP on the VPU, it outputs the video stream to the Host APP as a standard UVC camera.
* At the same time, it supports configuration, directly connect the video stream after ISP on the camera to the local inference pipeline of the camera, and output the inference results to the host app. This mode avoids downloading pictures to the inference engine, reduces the processing delay and saves bandwidth. The inference pipeline is configured through [openncc model JSON](https://eyecloudai.github.io/opennccframe/tutorials/how-to-write-json-config.html).

## Native API VPU Reference

More details please press [here](https://eyecloudai.github.io/opennccframe/native-api/index.html) .



---
[1]:../docs/mainres/ncsmode.png
[2]:../docs/mainres/mulitincs.png
[3]:../docs/mainres/uvcwithncs.png
[4]:../docs/mainres/aicamwithncs.png

