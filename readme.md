## OpenNCC Frame
In support of  [OpenNCC API 1.0]( https://github.com/EyecloudAi/openncc ) developers,we find the difficulties of edge intelligent engineers in product deployment and mass production! We have been thinking about how to make our developers have a set of cross platform unified programming interfaces in the face of fragmented scenarios, differentiated in-depth learning programming interfaces and customized hardware reasoning platforms, so that developers can pay more attention to their own business scenarios. This is a problem faced by our developer community and also [OpenNCC](www.openncc.com) team. According to the actual situation of our team and the feedback of community developers, we decided to build our frame on the basis of [GStreamer]( https://gstreamer.freedesktop.org/)  and [NNStreamer](https://github.com/nnstreamer/nnstreamer), so that [GStreamer]( https://gstreamer.freedesktop.org/) to support inference media pipelines, let [NNStreamer]( https://github.com/nnstreamer/nnstreamer) to support cross platform inference, the openncc team has more energy to support the product hardware platform and deep learning application layer support.    

The block diagram of openncc frame system is as follows  
![Top View Of OpenNCC Frame][1]  

 Locally, it can be divided into four layers from bottom to top:

- Platform driver layer  
  Provide driver support for AI inference acceleration on different hardware platforms and operating systems  
- GStreamer plug-in support layer  
  Various inference frameworks and accelerators (such as openncc native SDK) are encapsulated into plug-ins according to nnstreamer, and business supporting modules (such as statistics and tracking) are encapsulated into GStreamer standard plug-ins to provide a plug-in warehouse for flexible business scenarios  
- Media and inference pipeline layer  
  According to the business strategy, the plug-in is combined into a inference pipeline to output events, AI results and statistical information for the upper business  
- Business application and interaction layer  
  Realize business interaction, cloud docking and local report functions  



## Getting Started

For more details, please press [here](https://eyecloudai.github.io/openncc_frame/getting-started.html).


---

<font color="blue">**Note**</font>
Developers who are not familiar with the GStreamer framework can develop based on the native SDK. But we suggest that developers start their work from the GStreamer framework.

---



## Applications

* [Native examples](https://github.com/EyecloudAi/openncc_frame/tree/main/native_vpu_api/example)
* [NNStreamer example](https://github.com/EyecloudAi/openncc_frame/tree/main/nnstreamer/example)



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
If you need any help, please open an issue on Git Issues,or leave message on [openncc.com](https://www.openncc.com/forum). You are welcome to contact us for your suggestions, and model request.  



---
[1]: ./docs/mainres/frametopview.png
