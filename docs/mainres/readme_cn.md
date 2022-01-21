## OpenNCC Frame
  在支持我们的[OpenNCC API 1.0](https://github.com/EyecloudAi/openncc)开发者的过程中，发现了边缘智能工程师产品落地的痛！我们一直在思考如何让我们的开发者在面对碎片化场景，差异化深度学习编程接口和定制化的硬件推理平台时能够有一套跨平台的统一编程接口，让开发者有更多的精力关注到自己的业务场景上来。这是我们开发者社区面临的问题，也是我们团队一样面临的问题，在[OpenNCC API 1.0](https://github.com/EyecloudAi/openncc)时，我们的出发点是硬件积木化、软件容器化，在openncc frame我想我的出发点是软硬件一起积木化。根据我们团队的实际情况和社区开发者的反馈，我们决定在GStreamer和NNStreamer的基础上来构建我们的目标，让[GStreamer](https://gstreamer.freedesktop.org/)来支持积木化和推理媒体管道，让[NNStreamer](https://github.com/nnstreamer/nnstreamer)来支持跨平台推理，OpenNCC团队就有更多精力投入到支持产品化的硬件平台、深度学习应用层支持中。  
  我们规划的OpenNCC Frame系统框图如下：  
![Top View Of OpenNCC Frame][1]  
  在本地从下往上可以分为四层：  

- 平台驱动层  
  为AI推理加速在不同硬件平台和操作系统提供驱动支持  
  
- GStreamer插件支持层  
  将各种推理框架和推理加速(如：OpenNCC Native SDK)按照NNStreamer封装成插件，和业务提供支撑的模块(如：统计、跟踪)封装成GStreamer标准插件，为灵活的业务场景提供插件仓库  
  
- 媒体与推理管道层  

  根据业务策略将插件组合成推理管道，为上层业务输出事件、推理结果、统计信息

- 业务应用与交互层 
  
   实现业务交互、云端对接、本地报表功能

## OpenNCC Native SDK
  OpenNCC Native SDK是[OpenNCC API 1.0](https://github.com/EyecloudAi/openncc)的升级版本，为OpenNCC USB camera和OpenNCC SoM 加速卡提供跨平台驱动支持，管理和分配推理引擎资源，包括不限于：X86:Ubuntu,RK3568,Raspberry Pi,Nvidia. OpenNCC Native SDK已被封装成NNStreamer的子插件，为推理媒体管道提供底层资源调度接口，是OpenNCC Frame支持OpenNCC基于Intel MA2480 VPU硬件产品的专有开发包.  
  OpenNCC Native SDK提供如下工作模式：

### USB AI Acceleration Card(As Intel NCS) mode

![ncs mode][4]

* 加速卡模式下，Host APP从外部获取视频流(本地文件、IPC、webcam、V4L2 MIPI Cam),根据推理模型输入文件分辨率和格式需要配置预处理模块，通过OpenNCC Native SDK将图片发送给OpenNCC SoM推理，并将推理结果返回，推理支持异步、同步模式。OpenNCC SoM上的推理管道通过OpenNCC model Json(TODO:链接)配置完成。

* OpenNCC SoM本地最大支持6路推理管道配置，实时2路管道并发运行。用户可以通过Host APP中间处理来实现推理的多级串链或多模型并发。

  ![multi Acce Cards ][5]

  * 多个加速卡也被支持，用户根据算力需要可以扩增加速卡数量，SDK会动态分配算力。

### UVC Web Camera with AI Acceleration Card(As Intel NCS) mode   

![UVC with NCS][2]

*  这个模式下，OpenNCC是USB Camera和推理极速卡的组合体，前面接入OpenNCC支持的HD,4K Sensor,在VPU上完成ISP后作为标准的UVC摄像头输出视频流给上位机。
*  上位机获取到视频流后，进行预处理，根据推理模型输入文件分辨率和格式需要配置预处理模块，通过OpenNCC Native SDK将图片发送给OpenNCC SoM推理，并将推理结果返回，推理支持异步、同步模式。OpenNCC SoM上的推理管道通过OpenNCC model Json(TODO:链接)配置完成。

### UVC AI Camera with AI Acceleration Card(As Intel NCS) mode

![ai cam with ncs mode][3]

* 这个模式下，OpenNCC是USB Camera和推理极速卡的组合体，前面接入OpenNCC支持的HD,4K Sensor,在VPU上完成ISP后作为标准的UVC摄像头输出视频流给上位机。

* 同时支持配置，将摄像机上完成ISP后的视频流直接，接入相机本地推理管道，并输出推理结果给Host APP.这个模式避免了将图片下载给推理引擎，减少了处理延时，节省了带宽。推理管道通过OpenNCC model Json(TODO:链接)配置完成。

  

### 提示：

* 不喜欢GStreamer框架的开发者，可以基于Native SDK进行开发,我们建议开发者从GStreamer框架来开始他的工作。



## Getting Started

TODO



## Applications
* TODO: 链接到example下文档  
* TODO: 链接到nnstreamer的example  



## AI Acceleration Hardware Support
- OpenNCC USB powered by Intel Movidius VPU MA2480 :Released  
- Raspberry Pi with OpenNCC SoM:Released  
- RK3568 with OpenNCC SoM:Released  
- OpenNCC KMB powered by Intel Movidius KeamBay  
- OpenNCC Plus powered by NXP i.mx8mini Plus   

The following are supported by nnstreamer subplugins and are also supported:  
- Movidius-X via ncsdk2 subplugin: Released  
- Movidius-X via openVINO subplugin: Released  
- Edge-TPU via edgetpu subplugin: Released  
- ONE runtime via nnfw(an old name of ONE) subplugin: Released
- ARMNN via armnn subplugin: Released
- Verisilicon-Vivante via vivante subplugin: Released
- Qualcomm SNPE via snpe subplugin: Released
- Exynos NPU: WIP





## Get Support









[1]: ./frametopview.png
[2]:./uvcwithncs.png
[3]:./aicamwithncs.png
[4]:./ncsmode.png
[5]:./mulitincs.png

