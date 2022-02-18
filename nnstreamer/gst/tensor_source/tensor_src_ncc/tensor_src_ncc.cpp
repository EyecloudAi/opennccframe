/**
 * GStreamer Tensor_Src_TizenSensor
 * Copyright (C) 2019 MyungJoo Ham <myungjoo.ham@samsung.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 */
/**
 * @file    tensor_src_ncc.cpp
 * @date    16 Dec 2021
 * @brief   NNStreamer tensor-filter ncc subplugin template
 * @see     http://github.com/nnsuite/nnstreamer
 * @author  Zed <zhangdi@eyecloud.tech>
 * @bug     No known bugs
 */

/**
 * SECTION:element-tensor_src_ncc
 *
 * #tensor_src_ncc extends #gstbasesrc source element to handle Tizen
 * Sensor-Framework (sensord) as input.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m tensor_src_ncc type=ACCELEROMETER sequence=0 mode=POLLING ! fakesink
 * ]|
 * </refsect2>
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <gst/gst.h>
#include <glib.h>

#include <tensor_typedef.h>
#include <nnstreamer_plugin_api.h>
#include <nnstreamer_log.h>
#include <nnstreamer_util.h>

#include "tensor_src_ncc.h"

/**
 * @brief Macro for debug mode.
 */
#ifndef DBG
#define DBG (!self->silent)
#endif

/**
 * @brief Macro for debug message.
 */
#define silent_debug(...) do { \
    if (DBG) { \
      GST_DEBUG_OBJECT (self, __VA_ARGS__); \
    } \
  } while (0)

/**
 * @brief Free memory
 */
#define g_free_const(x) g_free((void*)(long)(x))
#define g_strfreev_const(x) g_strfreev((gchar**)(x))

GST_DEBUG_CATEGORY_STATIC (gst_tensor_src_ncc_debug);
#define GST_CAT_DEFAULT gst_tensor_src_ncc_debug

/**
 * @brief tensor_src_ncc properties.
 */
enum
{
  PROP_0,
  PROP_SILENT,
  PROP_MODEL,
  PROP_CONFIG,
  PROP_FREQ
};

/**
 * @brief Flag to print minimized log.
 */
#define DEFAULT_PROP_SILENT TRUE

/**
 * @brief Default Tizen sensor type
 */
#define DEFAULT_PROP_TYPE -1    /* Denotes "ALL" (any) */

/**
 * @brief Default sensor value retrieving mode
 */
#define DEFAULT_PROP_MODE "polling"

/**
 * @brief Default sensor retrieving frequency
 */
#define DEFAULT_PROP_FREQ_N 15
#define DEFAULT_PROP_FREQ_D 1

/**
 * @brief Default sequence number
 */
#define DEFAULT_PROP_SEQUENCE -1

#define _LOCK(obj) g_mutex_lock (&(obj)->lock)
#define _UNLOCK(obj) g_mutex_unlock (&(obj)->lock)

/** GObject method implementation */
static void gst_tensor_src_ncc_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_tensor_src_ncc_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);
static void gst_tensor_src_ncc_finalize (GObject * object);

/** GstBaseSrc method implementation */
static gboolean gst_tensor_src_ncc_start (GstBaseSrc * src);
static gboolean gst_tensor_src_ncc_stop (GstBaseSrc * src);
static gboolean gst_tensor_src_ncc_event (GstBaseSrc * src,
    GstEvent * event);
static gboolean gst_tensor_src_ncc_set_caps (GstBaseSrc * src,
    GstCaps * caps);
static GstCaps *gst_tensor_src_ncc_get_caps (GstBaseSrc * src,
    GstCaps * filter);
static GstCaps *gst_tensor_src_ncc_fixate (GstBaseSrc * src,
    GstCaps * caps);
static gboolean gst_tensor_src_ncc_is_seekable (GstBaseSrc * src);
static gboolean gst_tensor_src_ncc_query (GstBaseSrc * src, GstQuery * query);
static GstFlowReturn gst_tensor_src_ncc_create (GstBaseSrc * src,
    guint64 offset, guint size, GstBuffer ** buf);
static GstFlowReturn gst_tensor_src_ncc_fill (GstBaseSrc * src,
    guint64 offset, guint size, GstBuffer * buf);

/** internal functions */

#define gst_tensor_src_ncc_parent_class parent_class
G_DEFINE_TYPE (GstTensorSrcNcc, gst_tensor_src_ncc,
    GST_TYPE_BASE_SRC);

/**
 * @brief initialize the tensor_src_ncc class.
 */
static void
gst_tensor_src_ncc_class_init (GstTensorSrcNccClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GstElementClass *gstelement_class = (GstElementClass *) klass;
  GstBaseSrcClass *gstbasesrc_class = (GstBaseSrcClass *) klass;
  GstPadTemplate *pad_template;
  GstCaps *pad_caps;

  gobject_class->set_property = gst_tensor_src_ncc_set_property;
  gobject_class->get_property = gst_tensor_src_ncc_get_property;
  gobject_class->finalize = gst_tensor_src_ncc_finalize;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent",
          "Produce verbose output", DEFAULT_PROP_SILENT,
          (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
  g_object_class_install_property (gobject_class, PROP_MODEL,
      g_param_spec_string ("model", "Model filepath",
          "File path to the model file. Separated with ',' in case of multiple model files(like caffe2)",
          "", (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
  g_object_class_install_property (gobject_class, PROP_CONFIG,
      g_param_spec_string ("configure", "Configure filepath",
          "File path to the configure file.",
          "", (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  /* pad template */
  /** @todo Narrow down allowed tensors/tensor. */
  pad_caps = gst_caps_from_string (GST_TENSOR_CAP_DEFAULT "; "
      GST_TENSORS_CAP_WITH_NUM ("1"));
  pad_template = gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
      pad_caps);
  gst_element_class_add_pad_template (gstelement_class, pad_template);
  gst_caps_unref (pad_caps);

  gst_element_class_set_static_metadata (gstelement_class,
      "TensorSrcNcc", "Source/Tensor/Device",
      "Creates tensor(s) stream from a given OpenNCC",
      "Zed <zhangdi@eyecloud.tech>");

  gstbasesrc_class->set_caps =
      GST_DEBUG_FUNCPTR (gst_tensor_src_ncc_set_caps);
  gstbasesrc_class->get_caps =
      GST_DEBUG_FUNCPTR (gst_tensor_src_ncc_get_caps);
  gstbasesrc_class->fixate =
      GST_DEBUG_FUNCPTR (gst_tensor_src_ncc_fixate);
  gstbasesrc_class->is_seekable =
      GST_DEBUG_FUNCPTR (gst_tensor_src_ncc_is_seekable);
  gstbasesrc_class->start =
      GST_DEBUG_FUNCPTR (gst_tensor_src_ncc_start);
  gstbasesrc_class->stop = GST_DEBUG_FUNCPTR (gst_tensor_src_ncc_stop);
  gstbasesrc_class->query = GST_DEBUG_FUNCPTR (gst_tensor_src_ncc_query);
  gstbasesrc_class->create =
      GST_DEBUG_FUNCPTR (gst_tensor_src_ncc_create);
  gstbasesrc_class->fill = GST_DEBUG_FUNCPTR (gst_tensor_src_ncc_fill);
  gstbasesrc_class->event =
      GST_DEBUG_FUNCPTR (gst_tensor_src_ncc_event);
}

/**
 * @brief initialize tensor_src_ncc element.
 */
static void
gst_tensor_src_ncc_init (GstTensorSrcNcc * src)
{
  /** init properties */
  src->configured = FALSE;
  src->silent = DEFAULT_PROP_SILENT;
  src->running = FALSE;
  src->freq_n = DEFAULT_PROP_FREQ_N;
  src->freq_d = DEFAULT_PROP_FREQ_D;

  g_mutex_init (&src->lock);

  /**
   * @todo TBD. Update This!
   * format of the source since IIO device as a source is live and operates
   * at a fixed frequency, GST_FORMAT_TIME is used
   */
  gst_base_src_set_format (GST_BASE_SRC (src), GST_FORMAT_TIME);
  /** set the source to be a live source */
  gst_base_src_set_live (GST_BASE_SRC (src), TRUE);
  /** set base_src to automatically timestamp outgoing buffers
   * based on the current running_time of the pipeline.
   */
  gst_base_src_set_do_timestamp (GST_BASE_SRC (src), TRUE);
  /**
   * set async is necessary to make state change async
   * sync state changes does not need calling _start_complete() from _start()
   */
  gst_base_src_set_async (GST_BASE_SRC (src), TRUE);

  /** @todo TBD. Let's assume each frame has a fixed size */
  gst_base_src_set_dynamic_size (GST_BASE_SRC (src), FALSE);
}

/**
 * @brief This cleans up.
 * @details This cleans up the Tizen sensor handle/context,
 *          ready for a new handle/context or exit.
 *          This does not alter saved properties.
 * @returns it returns -1 if there is an error.
 */
static int
_ts_clean_up_handle (GstTensorSrcNcc * src)
{
  src->running = FALSE;
  src->configured = FALSE;
  return 0;
}

/**
 * @brief Parse the string of model
 * @param[out] prop Struct containing the properties of the object
 * @param[in] model_files the prediction model paths
 */
static void
gst_tensor_source_parse_modelpaths_string (NccTensorSrcProperties * prop,
    const gchar * model_files)
{
  if (prop == NULL)
    return;

  g_strfreev_const (prop->model_files);

  if (model_files) {
    prop->model_files = (const gchar **) g_strsplit_set (model_files, ",", -1);
    prop->num_models = g_strv_length ((gchar **) prop->model_files);
  } else {
    prop->model_files = NULL;
    prop->num_models = 0;
  }
}

/** @brief Handle "PROP_MODEL" for set-property */
static gint
_gtfc_setprop_MODEL (GstTensorSrcNcc * src, NccTensorSrcProperties * prop, const GValue * value)
{
  gint status = 0;
  const gchar *model_files = g_value_get_string (value);
  NccTensorSrcProperties _prop;

  if (!model_files) {
    ml_loge ("Invalid model provided to the tensor-source.");
    return 0;
  }
  _prop.model_files = NULL;

  gst_tensor_source_parse_modelpaths_string (prop, model_files);

  return 0;
}

/**
 * @brief Calculate interval in ms from framerate
 * @details This is effective only for TZN_SENSOR_MODE_ACTIVE_POLLING.
 */
static unsigned int
_ts_get_interval_ms (GstTensorSrcNcc * self)
{
  if (self->freq_n == 0)
    return 100;                 /* If it's 0Hz, assume 100ms interval */

  g_assert (self->freq_d > 0 && self->freq_n > 0);

  return gst_util_uint64_scale_int ((guint64) self->freq_d, 1000, self->freq_n);
}

/**
 * @brief Get handle, setup context, make it ready!
 */
static int
_ts_configure_handle (GstTensorSrcNcc * self)
{
  int ret = 0;
  NccTensorSpec_t tensor_desc_input;
  NccTensorSpec_t tensor_desc_output;

  gchar **str_model;
  guint num_path = 0;

  const GstTensorInfo *val;
  bool supported = false;

  /* 3. Configure interval_ms */
  self->interval_ms = _ts_get_interval_ms (self);

  nns_logi ("Set interval: %ums", self->interval_ms);

  /** Initialize your own framework or hardware here */
  int devNum = ncc_dev_number_get();
  nns_logd("ncc_dev_number_get find devs %d\n", devNum);
  if(devNum<=0)
  {
      nns_logi("ncc_dev_number_get error, the device was not found\n");
  }

  //load firmware
  ret = ncc_dev_init((char*)"/usr/local/lib/openncc/OpenNcc.mvcmd", devNum);
  if(ret<0)
  {
      nns_logf("ncc_dev_init error\n");
      return -1;
  }
  else
  {
      nns_logd("\ncc_dev_init success num %d\n",ret);
  }

  nns_logd("path %s\n",self->prop.model_files[0]);
  nns_logd("path %s\n",self->prop.config_files);

  //get parameter of handle
  if ((self->prop.num_models > 0) && (self->prop.config_files != NULL))
  {
      str_model = g_strsplit_set (self->prop.model_files[0], "/.", -1);
      num_path = g_strv_length (str_model);
      strcpy(self->handle.pipe_name , str_model[num_path-2]);
      printf("pipe_name %s\n",str_model[num_path-2]);

      strcpy(self->handle.model_path , self->prop.model_files[0]);
      if(self->prop.config_files != NULL){
          strcpy(self->handle.json_path, self->prop.config_files);
      }
      else{
          nns_logw("json config missed, model:%s\n",self->prop.model_files[0]);
          return -1;
      }
  }
  else{
      nns_logw("NULL blob path or config path\n");
      return -1;
  }

  ret = ncc_pipe_create(&self->handle, NCC_ASYNC);
  if(ret<0)
  {
      nns_logw("ncc_pipe_create error %d !\n", ret);
      return -1;
  }

  /* start device */
  ret = ncc_dev_start(0);
  if(ret<0)
  {
      nns_logf("ncc_dev_start error %d !\n", ret);
      return -1;
  }

  //get tensor descriptor
  ret = ncc_input_tensor_descriptor_get(&self->handle, &tensor_desc_input);
  if(ret<0)
  {
      nns_logf("ncc_input_tensor_descriptor_get error %d !\n", ret);
      return -1;
  }

  ret = ncc_output_tensor_descriptor_get(&self->handle, &tensor_desc_output);
  if(ret<0)
  {
      nns_logf("ncc_output_tensor_descriptor_get error %d !\n", ret);
      return -1;
  }

  self->src_spec.type = _NNS_FLOAT32;
  for(int i=0; i<4; i++)
  {
      self->src_spec.dimension[i] = tensor_desc_output.tensor[3-i];
  }

  self->configured = TRUE;

  return 0;
}

/**
 * @brief Keeping the handle/context, reconfigure a few parameters
 */
static int
_ts_reconfigure (GstTensorSrcNcc * self)
{
  _ts_clean_up_handle (self);
  return _ts_configure_handle (self);
}

/**
 * @brief set tensor_src_ncc properties
 */
static void
gst_tensor_src_ncc_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstTensorSrcNcc *self = GST_TENSOR_SRC_NCC (object);
  gint ret = 0;
  NccTensorSrcProperties *prop;
  UNUSED (pspec);

  prop= &self->prop;

  switch (prop_id) {
    case PROP_SILENT:
      self->silent = g_value_get_boolean (value);
      silent_debug ("Set silent = %d", self->silent);
      break;
    case PROP_MODEL:
      ret = _gtfc_setprop_MODEL (self, prop, value);
      break;
    case PROP_CONFIG:
      self->prop.config_files = g_value_dup_string(value);
      break;
    case PROP_FREQ:
    {
      gint n = self->freq_n;
      gint d = self->freq_d;

      _LOCK (self);

      self->freq_n = gst_value_get_fraction_numerator (value);
      self->freq_d = gst_value_get_fraction_denominator (value);

      if (self->freq_n < 0)
          self->freq_n = 0;
      if (self->freq_d < 1)
          self->freq_d = 1;

      silent_debug ("Set operating frequency %d/%d --> %d/%d",
          n, d, self->freq_n, self->freq_d);

      if (n != self->freq_n || d != self->freq_d) {
        /* Same sensor is kept. Only frequency is changed */
        if (ret) {
            self->freq_n = n;
            self->freq_d = d;
          GST_ERROR_OBJECT (self,
              "Calling _ts_reconfigure at set PROP_FREQ has failed. _ts_reconfigure () returns %d",
              ret);
        }
      }
      _UNLOCK (self);
    }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/**
 * @brief get tensor_src_ncc properties
 */
static void
gst_tensor_src_ncc_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstTensorSrcNcc *self = GST_TENSOR_SRC_NCC (object);
  NccTensorSrcProperties *prop;
  UNUSED (pspec);

  prop = &self->prop;

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, self->silent);
      break;
    case PROP_MODEL:
    {
      GString *gstr_models = g_string_new (NULL);
      gchar *models;
      int idx;

      /* return a comma-separated string */
      for (idx = 0; idx < prop->num_models; ++idx) {
        if (idx != 0) {
          g_string_append (gstr_models, ",");
        }

        g_string_append (gstr_models, prop->model_files[idx]);
      }

      models = g_string_free (gstr_models, FALSE);
      g_value_take_string (value, models);
      break;
    }
    case PROP_CONFIG:
    {
        g_value_set_string (value,
            (prop->config_files != NULL) ? prop->config_files : "");
        break;
    }
    case PROP_FREQ:
      gst_value_set_fraction (value, self->freq_n, self->freq_d);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/**
 * @brief finalize the instance
 */
static void
gst_tensor_src_ncc_finalize (GObject * object)
{
  GstTensorSrcNcc *self = GST_TENSOR_SRC_NCC (object);

  _LOCK (self);

  _ts_clean_up_handle (self);

  _UNLOCK (self);
  g_mutex_clear (&self->lock);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}


/**
 * @brief start function
 * @details This is called when state changed null to ready.
 *          load the device and init the device resources
 *          We won't configure before start is called.
 *          Postcondition: configured = TRUE. src = RUNNING.
 */
static gboolean
gst_tensor_src_ncc_start (GstBaseSrc * src)
{
    g_warning("\n\n-------------\n%s,%d \n",__FUNCTION__,__LINE__);
    int ret = 0;
    GstTensorSrcNcc *self = GST_TENSOR_SRC_NCC_CAST (src);
    gboolean retval = TRUE;
    guint blocksize;

    _LOCK (self);

    /* 1. Clean it up if there is a previous session */
    if (self->configured) {
      ret = _ts_clean_up_handle (self);
      if (ret) {
        GST_ERROR_OBJECT (self,
            "Start method failed, cleaning up previous context failed. _ts_clean_up_handle () returns %d",
            ret);
        retval = FALSE;           /* FAIL! */
        goto exit;
      }
    }

    /* 2. Configure handle / context */
    ret = _ts_configure_handle (self);
    if (ret) {
      retval = FALSE;
      goto exit;
    }
    g_assert (self->configured);

    printf("parent %d\n",self->handle.parent);
    /* set data size */
    blocksize = gst_tensor_info_get_size (&self->src_spec);
    gst_base_src_set_blocksize (src, blocksize);

    self->running = TRUE;

    /** complete the start of the base src */
    gst_base_src_start_complete (src, GST_FLOW_OK);

exit:
  _UNLOCK (self);
  return retval;
}

/**
 * @brief stop function.
 * @details This is called when state changed ready to null.
 *          Postcondition: configured = FALSE. src = STOPPED.
 */
static gboolean
gst_tensor_src_ncc_stop (GstBaseSrc * src)
{
    g_warning("\n\n-------------\n%s,%d \n",__FUNCTION__,__LINE__);
    int ret = 0;
    GstTensorSrcNcc *self = GST_TENSOR_SRC_NCC_CAST (src);
    gboolean retval = TRUE;

    _LOCK (self);

    ret = _ts_clean_up_handle (self);
    if (ret) {
      GST_ERROR_OBJECT (self,
          "Stop method failed, cleaning up previous context failed. _ts_clean_up_handle () returns %d",
          ret);
      retval = FALSE;             /* FAIL! */
      goto exit;
    }

    g_assert (!self->configured);

  exit:
    _UNLOCK (self);
    return retval;
}

/**
 * @brief handle events
 */
static gboolean
gst_tensor_src_ncc_event (GstBaseSrc * src, GstEvent * event)
{
    /** No events to be handled yet */
    return GST_BASE_SRC_CLASS (parent_class)->event (src, event);
}

/**
 * @brief Get possible GstCap from the configuration of src.
 */
static GstCaps *
_ts_get_gstcaps_from_conf (GstTensorSrcNcc * self)
{
    const GstTensorInfo *spec;
    GstCaps *retval;

    spec = &self->src_spec;

    if (!self->configured || NULL == spec) {
      retval = gst_pad_get_pad_template_caps (GST_BASE_SRC_PAD (self));
    } else {
      GstTensorsConfig tensors_config;

      gst_tensors_config_init (&tensors_config);
      tensors_config.info.num_tensors = 1;

      gst_tensor_info_copy (&tensors_config.info.info[0], spec);
      tensors_config.rate_n = self->freq_n;
      tensors_config.rate_d = self->freq_d;

      retval = gst_tensor_caps_from_config (&tensors_config);
      gst_caps_append (retval, gst_tensors_caps_from_config (&tensors_config));
    }

    return retval;
}

/**
 * @brief set new caps
 * @retval TRUE if it's acceptable. FALSE if it's not acceptable.
 */
static gboolean
gst_tensor_src_ncc_set_caps (GstBaseSrc * src, GstCaps * caps)
{
    GstTensorSrcNcc *self = GST_TENSOR_SRC_NCC_CAST (src);
    GstCaps *cap_tensor;
    gboolean retval = FALSE;

    _LOCK (self);

    cap_tensor = _ts_get_gstcaps_from_conf (self);

    /* Check if it's compatible with either tensor or tensors */
    retval = gst_caps_can_intersect (caps, cap_tensor);
    gst_caps_unref (cap_tensor);

    _UNLOCK (self);
    return retval;
}

/**
 * @brief get caps of subclass
 * @note basesrc _get_caps returns the caps from the pad_template
 * however, we set the caps manually and needs to returned here
 */
static GstCaps *
gst_tensor_src_ncc_get_caps (GstBaseSrc * src, GstCaps * filter)
{
    GstCaps *caps;
    GstPad *pad;

    pad = src->srcpad;
    caps = gst_pad_get_current_caps (pad);

    if (caps == NULL)
      caps = gst_pad_get_pad_template_caps (pad);

    if (filter) {
      GstCaps *intersection =
          gst_caps_intersect_full (filter, caps, GST_CAPS_INTERSECT_FIRST);
      gst_caps_unref (caps);
      caps = intersection;
    }

    return caps;
}

/**
 * @brief fixate the caps when needed during negotiation
 */
static GstCaps *
gst_tensor_src_ncc_fixate (GstBaseSrc * src, GstCaps * caps)
{
    GstTensorSrcNcc *self = GST_TENSOR_SRC_NCC_CAST (src);
    GstCaps *cap_tensor;
    GstCaps *retval = NULL;

    _LOCK (self);

    cap_tensor = _ts_get_gstcaps_from_conf (self);

    if (gst_caps_can_intersect (caps, cap_tensor))
      retval = gst_caps_intersect (caps, cap_tensor);
    gst_caps_unref (cap_tensor);

    _UNLOCK (self);
    return gst_caps_fixate (retval);
}

/**
 * @brief Sensor nodes are not seekable.
 */
static gboolean
gst_tensor_src_ncc_is_seekable (GstBaseSrc * src)
{
    nns_logd ("tensor_src_ncc is not seekable");
    return FALSE;
}

/**
 * @brief Handle queries.
 *
 * GstBaseSrc method implementation.
 */
static gboolean
gst_tensor_src_ncc_query (GstBaseSrc * src, GstQuery * query)
{
    gboolean res = FALSE;
    GstTensorSrcNcc *self = GST_TENSOR_SRC_NCC_CAST (src);

    switch (GST_QUERY_TYPE (query)) {
      case GST_QUERY_LATENCY:
      {
        GstClockTime min_latency, max_latency = -1;
        gint freq_d, freq_n;

        freq_d = self->freq_d;
        freq_n = self->freq_n;

        /* we must have a framerate */
        if (freq_n <= 0 || freq_d <= 0) {
          GST_WARNING_OBJECT (self,
              "Can't give latency since framerate isn't fixated");
          goto done;
        }

        /* min latency is the time to capture one frame/field */
        min_latency = gst_util_uint64_scale_int (GST_SECOND, freq_d, freq_n);

        GST_DEBUG_OBJECT (self,
            "Reporting latency min %" GST_TIME_FORMAT " max %" GST_TIME_FORMAT,
            GST_TIME_ARGS (min_latency), GST_TIME_ARGS (max_latency));

        gst_query_set_latency (query, TRUE, min_latency, max_latency);

        res = TRUE;
        break;
      }
      default:
        res = GST_BASE_SRC_CLASS (parent_class)->query (src, query);
        break;
    }

done:
    return res;
}

/**
 * @brief create a buffer with requested size and offset
 * @note offset, size ignored as the tensor src ncc does not support pull mode
 */
static GstFlowReturn
gst_tensor_src_ncc_create (GstBaseSrc * src, guint64 offset,
    guint size, GstBuffer ** buffer)
{
    GstTensorSrcNcc *self = GST_TENSOR_SRC_NCC_CAST (src);
    GstBuffer *buf = gst_buffer_new ();
    GstMemory *mem;
    guint buffer_size;
    GstFlowReturn retval = GST_FLOW_OK;

    _LOCK (self);

    if (!self->configured) {
      GST_ERROR_OBJECT (self,
          "Buffer creation requested while the element is not configured. gst_tensor_src_tizensensor_create() cannot proceed if it is not configured.");
      retval = GST_FLOW_ERROR;
      goto exit;
    }

    /* We don't have multi-tensor (tensors with num-tensors > 1) */
    buffer_size = gst_tensor_info_get_size (&self->src_spec);
    mem = gst_allocator_alloc (NULL, buffer_size, NULL);
    if (mem == NULL) {
      GST_ERROR_OBJECT (self,
          "Cannot allocate memory for gst buffer of %u bytes", buffer_size);
      retval = GST_FLOW_ERROR;
      goto exit;
    }
    gst_buffer_append_memory (buf, mem);

    _UNLOCK (self);
    retval = gst_tensor_src_ncc_fill (src, offset, buffer_size, buf);
    _LOCK (self);
    if (retval != GST_FLOW_OK)
      goto exit;

    *buffer = buf;

  exit:
    _UNLOCK (self);
    return retval;
}

/**
 * @brief fill the buffer with data
 * @note ignore offset,size as there is pull mode
 * @note buffer timestamp is already handled by gstreamer with gst clock
 * @note Get data from Tizen Sensor F/W. Get the timestamp as well!
 */
static GstFlowReturn
gst_tensor_src_ncc_fill (GstBaseSrc * src, guint64 offset,
    guint size, GstBuffer * buffer)
{
    GstTensorSrcNcc *self = GST_TENSOR_SRC_NCC_CAST (src);
    GstFlowReturn retval = GST_FLOW_OK;
    GstMemory *mem;
    GstMapInfo map;

    int count = 0;
    gint64 duration;
    int ret;
    int fd,ret_read;
    _LOCK (self);

    if (!self->configured) {
      GST_ERROR_OBJECT (self,
          "gst_tensor_src_ncc_fill() cannot proceed if it is not configured.");
      retval = GST_FLOW_ERROR;
      goto exit;
    }

    if (size != gst_tensor_info_get_size (&self->src_spec)) {
      GST_ERROR_OBJECT (self,
          "gst_tensor_src_ncc_fill() requires size value (%u) to be matched with the configurations of sensors (%lu)",
          size, (unsigned long) gst_tensor_info_get_size (&self->src_spec));
      retval = GST_FLOW_ERROR;
      goto exit;
    }

    mem = gst_buffer_peek_memory (buffer, 0);
    if (!gst_memory_map (mem, &map, GST_MAP_WRITE)) {
      GST_ERROR_OBJECT (self,
          "gst_tensor_src_ncc_fill() cannot map the given buffer for writing data.");
      retval = GST_FLOW_ERROR;
      goto exit;
    }

    /* 3. Set duration so that BaseSrc handles the frequency */
    if (self->freq_n == 0)
        /* 100ms */
        duration = 100 * 1000 * 1000;
    else
        duration = gst_util_uint64_scale_int (GST_SECOND,self->freq_d, self->freq_n);

    GST_BUFFER_DURATION (buffer) = duration;

    /* 4. Write values to buffer. Be careful on type casting */
    NccPipeOutput_t pOutData;
    pOutData.output = map.data;
    pOutData.alloc_size = size;
    ncc_pipe_queue_read(&(self->handle), &pOutData, 0);

exit_unmap:
    gst_memory_unmap (mem, &map);
exit:
    _UNLOCK (self);
    return retval;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  /* debug category for filtering log messages
   *
   * exchange the string 'Template plugin' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_tensor_src_ncc_debug, "ncctensorsrc",
      0, "OpenNCC Tensor Source");

  return gst_element_register (plugin, "ncctensorsrc", GST_RANK_NONE,
          GST_TYPE_TENSOR_SRC_NCC);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    ncctensorsrc,
    "ncc_tensor_src",
    plugin_init, PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
