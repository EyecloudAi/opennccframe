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
 */

/**
 * @file	tensor_src_ncc.h
 * @date	07 Nov 2019
 * @brief	GStreamer plugin to support Tizen sensor framework (sensord)
 * @see		https://github.com/nnstreamer/nnstreamer
 * @author	MyungJoo Ham <myungjoo.ham@samsung.com>
 * @bug		No known bugs except for NYI items
 */

#ifndef __GST_TENSOR_SRC_NCC_H__
#define __GST_TENSOR_SRC_NCC_H__

#include <gst/gst.h>
#include <gst/base/gstbasesrc.h>

#include <tensor_typedef.h> /* GstTensorInfo */
#include "native_vpu_api.h"

G_BEGIN_DECLS
#define GST_TYPE_TENSOR_SRC_NCC \
  (gst_tensor_src_ncc_get_type())
#define GST_TENSOR_SRC_NCC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TENSOR_SRC_NCC,GstTensorSrcNcc))
#define GST_TENSOR_SRC_NCC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TENSOR_SRC_NCC,GstTensorSrcNccClass))
#define GST_IS_TENSOR_SRC_NCC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TENSOR_SRC_NCC))
#define GST_IS_TENSOR_SRC_NCC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TENSOR_SRC_NCC))
#define GST_TENSOR_SRC_NCC_CAST(obj)  ((GstTensorSrcNcc *)(obj))
typedef struct _GstTensorSrcNcc GstTensorSrcNcc;
typedef struct _GstTensorSrcNccClass GstTensorSrcNccClass;

typedef struct
{
    const char **model_files;
    int num_models;

    const char *config_files;
}NccTensorSrcProperties;

/**
 * @brief GstTensorSrcNcc data structure.
 *
 * GstTensorSrcNcc inherits GstBaseSrcOpen.
 */
struct _GstTensorSrcNcc
{
  GstBaseSrc element; /**< parent class object */

  /** gstreamer related properties */
  gboolean silent; /**< true to print minimized log */
  gboolean configured; /**< true if device is configured and ready */
  gboolean running; /**< true if src is active and data is flowing */

  /** For managing critical sections */
  GMutex lock;

  /** Properties saved */
  gint freq_n; /**< Operating frequency of N/d */
  gint freq_d; /**< Operating frequency of n/D */

  NccTensorSrcProperties prop;
  NccPipeHandle_t handle;

  /**
   * Sensor node info (handle, context)
   * These are temporary values valid during a session of "configured"
   * values should be cleared when confiured becomes FALSE
   */
  GstTensorInfo src_spec;
  unsigned int interval_ms;
};

/**
 * @brief GstTensorSrcNCCClass data structure.
 *
 * GstTensorSrcNCC inherits GstBaseSrc.
 */
struct _GstTensorSrcNccClass
{
  GstBaseSrcClass parent_class; /**< inherits class object */
};

/**
 * @brief Function to get type of tensor_src_open.
 */
GType gst_tensor_src_open_get_type (void);

G_END_DECLS
#endif /** __GST_TENSOR_SRC_NCC_H__ */
