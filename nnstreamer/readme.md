## OpenNCC  Elements    
  * The directories  under gst prefixed with tensor_* are  plug-ins for NNStreams,others 
are the elements for the GStreamer.  

## How to buid and install the elements  
* Before install openncc's elements you need install  NNStreamer, [clik it](https://eyecloudai.github.io/opennccframe/getting-started/install-from-repo/install-ncc-elements-into-nnstreamer.html) 
to know how to install it. 
* Since we want the nnstreamer to support all the NCC devices,we need patch to nnstreamer,let the nnstreamer  tensor_filter could 
identify openncc devices.  
$ meson build  
$ sudo ninja -C build install  

## How to run examples
We need to create gstreamer pipeline for AI inference. C/C++ , Python  and  gst-launch CLI(command-line interface) are supported.  
## How to  build pipelines
* [gst-launch pipeline](https://gstreamer.freedesktop.org/documentation/tools/gst-launch.html?gi-language=c)     
* [GStreamer Plugs](https://gstreamer.freedesktop.org/documentation/plugins_doc.html)

## More nnstramer example run with OpenNCC Frame  
* [NNStreamer Examples](https://github.com/nnstreamer/nnstreamer-example/tree/main/native)  
* Usually we need init the pipeline like this:  
	

 	/* init pipeline */
 	 str_pipeline =
 	     g_strdup_printf
 	     ("v4l2src name=cam_src ! videoconvert ! videoscale ! "
 	  	"video/x-raw,width=640,height=480,format=RGB ! tee name=t_raw "
 	  	"t_raw. ! queue ! textoverlay name=tensor_res font-desc=Sans,24 ! "
 	  	"videoconvert ! ximagesink name=img_tensor "
 	"t_raw. ! queue leaky=2 max-size-buffers=2 ! videoscale ! tensor_converter ! "
 	"tensor_filter framework=tensorflow-lite model=%s ! "
 	"tensor_sink name=tensor_sink", g_app.tflite_info.model_path);  
 	
 	 We could switch the inference framewrok to OpenNCC,we need change the tensor_filter framework to 'ncc',and also give the path of the AI model with the openncc json config file. etc.:  
 > 'tensor_filter framework=ncc  model=/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/person-detection-retail-0013/person-detection-retail-0013.blob custom=/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/person-detection-retail-0013/config/input_BGR.json'  



## FAQ

* 1). using meson to build project failed  
If your meson version is older than 0.5,would meet issue like follow:
The Meson build system
Version: 0.45.1
Source dir: /home/firefly/work/nnstreamer
Build dir: /home/firefly/work/nnstreamer/build
Build type: native build
meson.build:309:10: ERROR: lexer
features = {
          ^
We could install new version:
$ pip3 install --user meson==0.50
$ export PATH=~/.local/bin:$PATH  

* 2). tools/development/parser/meson.build:5:0: ERROR: Program(s) ['flex', 'win_flex'] not found or not executable  
$ sudo apt-get install flex  bison  



