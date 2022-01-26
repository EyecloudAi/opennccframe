#!/usr/bin/env python

"""
@file		example_face_detection_ncc.py
@date		26 Jan. 2022
@brief		Tensor stream example with filter
@see		https://github.com/EyecloudAi/openncc_frame
@author		Zed <zhangdi@eyecloud.tech>
@bug		No known bugs.

NNStreamer example for face detection using openvino face-detection-retail-0004.

Pipeline :
compositor ── videoconvert ── xvimagesink  
v4l2src ── videorate ── videoscale ── videoconvert ──  tee  
tee ───  compositor.sink_0  
tee ───  videoconvert ── videoscale ── tensor_converter ── tensor_filter ── tensor_decoder ── compositor.sink_1  

This app displays video sink.

"""

import os
import sys
sys.path.append('/usr/lib/python3/dist-packages')
import logging
import gi
import argparse

gi.require_version('Gst', '1.0')
from gi.repository import Gst, GObject

class NNStreamerExample:
    """NNStreamer example for image classification."""

    def __init__(self, argv=None):
        self.loop = None
        self.pipeline = None
        self.running = False

        GObject.threads_init()
        Gst.init(argv)

        self.DEVICE = argv[1];
        """Init pipeline and run example.

        :return: None
        """
        # main loop
        self.loop = GObject.MainLoop()

        # init pipeline
        self.pipeline = Gst.parse_launch(
            'compositor name=mix sink_0::zorder=1 sink_1::zorder=2 ! videoconvert ! '
            'xvimagesink v4l2src device=' + str(self.DEVICE) + ' ! videorate ! videoconvert ! videoscale ! '
            'video/x-raw,width=1920,height=1080,format=YV12,framerate=15/1 ! tee name=t t. ! '
            'queue ! mix.sink_0  t. ! queue  ! videoconvert ! videoscale ! '
            'video/x-raw,height=300,width=300,format=BGR !         tensor_converter ! '
            'tensor_transform mode=typecast option=uint8 ! tensor_transform mode=dimchg option=0:2 !  '
            'tensor_filter framework=ncc  model=/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/face-detection-retail-0004.blob custom=/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/config/input_BGR.json silent=false accelerator=true  ! '
            'tensor_decoder mode=bounding_boxes option1=ov-person-detection option4=1920:1080 option5=300:300 ! mix.sink_1 '
        )

        # bus and message callback
        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.connect('message', self.on_bus_message)

        # tensor sink signal : new data callback
        #tensor_sink = self.pipeline.get_by_name('tensor_sink')
        #tensor_sink.connect('new-data', self.on_new_data)


        # start pipeline
        self.pipeline.set_state(Gst.State.PLAYING)
        self.running = True

        # set window title
        self.set_window_title('img_tensor', 'NNStreamer Example')

        # run main loop
        self.loop.run()

        # quit when received eos or error message
        self.running = False
        self.pipeline.set_state(Gst.State.NULL)

        bus.remove_signal_watch()

    def on_bus_message(self, bus, message):
        """Callback for message.

        :param bus: pipeline bus
        :param message: message from pipeline
        :return: None
        """
        if message.type == Gst.MessageType.EOS:
            logging.info('received eos message')
            self.loop.quit()
        elif message.type == Gst.MessageType.ERROR:
            error, debug = message.parse_error()
            logging.warning('[error] %s : %s', error.message, debug)
            self.loop.quit()
        elif message.type == Gst.MessageType.WARNING:
            error, debug = message.parse_warning()
            logging.warning('[warning] %s : %s', error.message, debug)
        elif message.type == Gst.MessageType.STREAM_START:
            logging.info('received start message')
        elif message.type == Gst.MessageType.QOS:
            data_format, processed, dropped = message.parse_qos_stats()
            format_str = Gst.Format.get_name(data_format)
            logging.debug('[qos] format[%s] processed[%d] dropped[%d]', format_str, processed, dropped)

    def set_window_title(self, name, title):
        """Set window title.

        :param name: GstXImageSink element name
        :param title: window title
        :return: None
        """
        element = self.pipeline.get_by_name(name)
        if element is not None:
            pad = element.get_static_pad('sink')
            if pad is not None:
                tags = Gst.TagList.new_empty()
                tags.add_value(Gst.TagMergeMode.APPEND, 'title', title)
                pad.send_event(Gst.Event.new_tag(tags))

    def tflite_init(self):
        """Check tflite model and load labels.

        :return: True if successfully initialized
        """
        return True

if __name__ == '__main__':
    example = NNStreamerExample(sys.argv)
