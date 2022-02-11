/**
 * GStreamer Tensor_Filter ncc Code
 * Copyright (C) 2019 MyungJoo Ham <myungjoo.ham@samsung.com>
 *
 * This is a template with no license requirements.
 * Writers may alter the license to anything they want.
 * The author hereby allows to do so.
 */
/**
 * @file	tensor_filter_ncc.cpp
 * @date	16 Dec 2021
 * @brief	NNStreamer tensor-filter ncc subplugin template
 * @see		http://github.com/nnsuite/nnstreamer
 * @author	Zed <zhangdi@eyecloud.tech>
 * @bug		No known bugs
 */
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gst/gst.h>
#define NO_ANONYMOUS_NESTED_STRUCT
#include <nnstreamer_plugin_api_filter.h>
#undef NO_ANONYMOUS_NESTED_STRUCT
#include <nnstreamer_util.h>
#include <nnstreamer_log.h>
#include "native_vpu_api.h"
#include "cameraCtrl.h"

static const gchar *ncc_accl_support[] = {
  ACCL_NPU_STR,
  ACCL_NPU_MOVIDIUS_STR,
  NULL
};

void init_filter_ncc (void) __attribute__ ((constructor));
void fini_filter_ncc (void) __attribute__ ((destructor));

#define NNC2_MAX_NUM_TENOSORS_SUPPORTED  1

/**
 * @brief If you need to store session or model data,
 *        this is what you want to fill in.
 */
typedef struct
{
    NccPipeHandle_t handle;
    NccTensorSpec_t tensor_desc_input;  /** description of input  tensor */
    NccTensorSpec_t tensor_desc_output; /** description of output tensor */
} ncc_pdata;

static void _ncc_close (const GstTensorFilterProperties * prop,
    void **private_data);

/**
 * @brief Check condition to reopen model.
 */
static int
_ncc_reopen (const GstTensorFilterProperties * prop, void **private_data)
{
  /**
   * @todo Update condition to reopen model.
   *
   * When called the callback 'open' with user data,
   * check the model file or other condition whether the model or framework should be reloaded.
   * Below is example: when model file is changed, return 1 to reopen model.
   */

    ncc_pdata *pdata = (ncc_pdata *)*private_data;

    if (prop->num_models > 0 && strcmp (prop->model_files[0], (const char*)pdata->handle.model_path) != 0) {
    return 1;
    }

    return 0;
}

/**
 * @brief The standard tensor_filter callback
 */
static int
_ncc_open (const GstTensorFilterProperties * prop, void **private_data)
{
    /* Variables of the data types from nccsdk.h */
    NccPipeHandle_t handle;
    NccTensorSpec_t tensor_desc_input;
    NccTensorSpec_t tensor_desc_output;
    /* Normal variables */
    ncc_pdata *pdata = NULL;
    gchar **str_model;
    guint num_path = 0;
    int ret = 0;

    if (*private_data != NULL) {
        if (_ncc_reopen (prop, private_data) != 0) {
          _ncc_close (prop, private_data);  /* "reopen" */
        } else {
          return 1;
        }
    }

    pdata = g_new0 (ncc_pdata, 1);
    if (pdata == NULL)
        return -1;

    /** Initialize your own framework or hardware here */
    int devNum = ncc_dev_number_get();
    nns_logd("ncc_dev_number_get find devs %d\n", devNum);
    if(devNum<=0)
    {
        nns_logi("ncc_dev_number_get error, the device was not found\n");
    }

    //load firmware
    ret = ncc_dev_init((char*)"/usr/lib/openncc/OpenNcc.mvcmd", devNum);
    if(ret<0)
    {
        nns_logf("ncc_dev_init error\n");
        return -1;
    }
    else
    {
        nns_logd("\ncc_dev_init success num %d\n",ret);
    }

    nns_logd("path %s\n",prop->model_files[0]);
    nns_logd("path %s\n",prop->custom_properties);

    //get parameter of handle
    if ((prop->num_models > 0) ||(prop->custom_properties != NULL))
    {
        if(prop->output_meta.info[0].name != NULL)
            sprintf(handle.pipe_name , prop->output_meta.info[0].name);
        else
        {
            str_model = g_strsplit_set (prop->model_files[0], "/.", -1);
            num_path = g_strv_length (str_model);
            strcpy(handle.pipe_name , str_model[num_path-2]);
            nns_logd("pipe_name %s\n",str_model[num_path-2]);
        }

        if(strlen(prop->model_files[0])<sizeof(handle.model_path))
            strcpy(handle.model_path , prop->model_files[0]);
        else
            nns_logf("model path too long.\n");

        if(prop->custom_properties != NULL){
            if(strlen(prop->custom_properties)<sizeof(handle.json_path))
                strcpy(handle.json_path, prop->custom_properties);
            else
                nns_logf("json config path too long.\n");
        }
        else{
            nns_logf("json config missed, model:%s\n",prop->model_files[0]);
            return -1;
        }
    }
    else{
        nns_logf("NULL blob path or config path\n");
        return -1;
    }

    /* create handle, synchronous mode */
    ret = ncc_pipe_create(&handle, NCC_SYNC);

    if(ret<0)
    {
        nns_logf("ncc_pipe_create error %d !\n", ret);
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
    ret = ncc_input_tensor_descriptor_get(&handle, &tensor_desc_input);
    if(ret<0)
    {
        nns_logf("ncc_input_tensor_descriptor_get error %d !\n", ret);
        return -1;
    }

    ret = ncc_output_tensor_descriptor_get(&handle, &tensor_desc_output);
    if(ret<0)
    {
        nns_logf("ncc_output_tensor_descriptor_get error %d !\n", ret);
        return -1;
    }

    pdata->handle = handle;
    pdata->tensor_desc_input = tensor_desc_input;
    pdata->tensor_desc_output = tensor_desc_output;
    *private_data = (void *) pdata;

    return 0;
}

/**
 * @brief The standard tensor_filter callback
 */
static void
_ncc_close (const GstTensorFilterProperties * prop, void **private_data)
{
    UNUSED(prop);
    ncc_pdata *pdata;
    pdata = (ncc_pdata *)*private_data;

    /** @todo Close what you have opened/allocated with ncc_open */
    g_free (pdata);
    *private_data = NULL;
}

/**
 * @brief The standard tensor_filter callback
 */
static int
_ncc_invoke (const GstTensorFilterProperties * prop,
    void **private_data, const GstTensorMemory * input,
    GstTensorMemory * output)
{
    ncc_pdata *pdata = (ncc_pdata *)*private_data;
    NccPipeHandle_t *handle = &(pdata->handle);

    int buf_size;
    int ret = 0;

    NccPipeInput_t *pInData = NULL;
    NccPipeOutput_t pOutData;

    g_return_val_if_fail (prop->input_configured, -1);
    if (prop->input_meta.num_tensors > NNC2_MAX_NUM_TENOSORS_SUPPORTED) {
        nns_logf ("The number of input tensor overflow\n");
        goto err_destroy;
    }

    /* allocates memory for storing images sent to the device */
    buf_size = (int)input->size;
    pInData = ncc_malloc(buf_size);
    if(pInData==0)
    {
        nns_logf("'nccMalloc error\n");
        return 0;
    }

    memcpy(pInData->input, input->data, input->size);
    pInData->seqNo = 0;

    /* allocates memory for storing meta data */
    pOutData.alloc_size = output->size;
    pOutData.output     = output->data;

    /* Invoke the synchronous inference interface，overtime is 10s */
    ret = sync_process(handle, pInData, &pOutData, 10000);
    nns_logd("%s, outSize:%d ,T:%d MS\n",pdata->handle.pipe_name, pOutData.actual_size, (int)pOutData.elapsed_time);

    if(ret!=0)
    {
        nns_logf("syncProc err, ret=%d\n", ret);
        return -1;
    }

    /* free memory */
    ncc_free(pInData);

    return 0;

err_destroy:
    _ncc_close (prop, private_data);
    return -1;
}

/**
 * @brief The standard tensor_filter callback for static input/output dimension.
 * @note If you want to support flexible/dynamic input/output dimension,
 *       read nnstreamer_plugin_api_filter.h and supply the
 *       setInputDimension callback.
 */
static int
_ncc_getInputDim (const GstTensorFilterProperties * prop,
    void **private_data, GstTensorsInfo * info)
{
  UNUSED(prop);
  ncc_pdata *pdata = (ncc_pdata *)*private_data;
  NccTensorSpec_t *ncc_input_desc = &(pdata->tensor_desc_input);
  GstTensorInfo *nns_input_tensor_info;

  info->num_tensors = NNC2_MAX_NUM_TENOSORS_SUPPORTED;
  nns_input_tensor_info =
      &(info->info[NNC2_MAX_NUM_TENOSORS_SUPPORTED - 1]);

  nns_input_tensor_info->type = _NNS_UINT8;//_NNS_UINT8; _NNS_FLOAT32
  for(int i=0; i<4; i++)
  {
      nns_input_tensor_info->dimension[i] = ncc_input_desc->tensor[3-i];
      nns_logd("tensor_info %d %d\n",i,nns_input_tensor_info->dimension[i]);
  }

  return 0;
}

/**
 * @brief The standard tensor_filter callback for static input/output dimension.
 * @note If you want to support flexible/dynamic input/output dimension,
 *       read nnstreamer_plugin_api_filter.h and supply the
 *       setInputDimension callback.
 */
static int
_ncc_getOutputDim (const GstTensorFilterProperties * prop,
    void **private_data, GstTensorsInfo * info)
{
    UNUSED(prop);
    ncc_pdata *pdata = (ncc_pdata *)*private_data;
    NccTensorSpec_t *ncc_output_desc = &(pdata->tensor_desc_output);
    GstTensorInfo *nns_output_tensor_info;

    info->num_tensors = NNC2_MAX_NUM_TENOSORS_SUPPORTED;
    nns_output_tensor_info =
        &(info->info[NNC2_MAX_NUM_TENOSORS_SUPPORTED - 1]);

    nns_output_tensor_info->type = _NNS_FLOAT32;//_NNS_UINT16;
    for(int i=0; i<4; i++)
    {
        nns_output_tensor_info->dimension[i] = ncc_output_desc->tensor[3-i];
        nns_logd("tensor_info %d %d\n",i,nns_output_tensor_info->dimension[i]);
    }

    return 0;
}

static int
_ncc_checkAvailability (accl_hw hw)
{
  if (g_strv_contains (ncc_accl_support, get_accl_hw_str (hw)))
    return 0;

  return -1;
}

static gchar filter_subplugin_ncc[] = "ncc";

static GstTensorFilterFramework NNS_support_ncc = {
    .version = GST_TENSOR_FILTER_FRAMEWORK_V0,
    .open = _ncc_open,
    .close = _ncc_close,
    {
        .v0 = {
            .name = filter_subplugin_ncc,
            .allow_in_place = FALSE,
            .allocate_in_invoke = FALSE,
            .run_without_model = FALSE,
            .verify_model_path = FALSE,
            .statistics = NULL,
            .invoke_NN = _ncc_invoke,
            .getInputDimension = _ncc_getInputDim,
            .getOutputDimension = _ncc_getOutputDim,
            .setInputDimension = NULL,
            .destroyNotify = NULL,
            .reloadModel = NULL,
            .handleEvent = NULL,
            .checkAvailability = _ncc_checkAvailability,
            .allocateInInvoke = NULL,
        }
    }
};

/**@brief Initialize this object for tensor_filter subplugin runtime register */
void
init_filter_ncc (void)
{
  nnstreamer_filter_probe (&NNS_support_ncc);
}

/** @brief Destruct the subplugin */
void
fini_filter_ncc (void)
{
  nnstreamer_filter_exit (NNS_support_ncc.v0.name);
}
