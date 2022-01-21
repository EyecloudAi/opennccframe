/**！
 * @file   native_vpu_api.h
 * @author Zed
 * @date   2021.11.15
 * @brief Configure inference engine and pipelines of NCC camera
 */
#ifndef  __NATIVE_VPU_API_H__
#define  __NATIVE_VPU_API_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**！
 * @define MAX_DEV_NUM
 * @brief The maximum of the ncc devices which the sdk supported by one host APP
*/
#define MAX_DEV_NUM        (2)

/**！
 * @define MAX_PIPELINE_NUM
 * @brief The maximum of the AI inference pipelines which one device supported
*/
#define MAX_PIPELINE_NUM   (6)

/**!
  * @enum usb_error
  * @brief The list of the usb error code
  */
enum usb_error {
    /** < Success (no error) */
    USB_SUCCESS = 0,

    /** < Input/output error */
    USB_ERROR_IO = -1,

    /** < Invalid parameter */
    USB_ERROR_INVALID_PARAM = -2,

    /**< Access denied (insufficient permissions) */
    USB_ERROR_ACCESS = -3,

    /**< No such device (it may have been disconnected) */
    USB_ERROR_NO_DEVICE = -4,

    /**< Entity not found */
    USB_ERROR_NOT_FOUND = -5,

    /**< Resource busy */
    USB_ERROR_BUSY = -6,

    /**< Operation timed out */
    USB_ERROR_TIMEOUT = -7,

    /**< Overflow */
    USB_ERROR_OVERFLOW = -8,

    /**< Pipe error */
    USB_ERROR_PIPE = -9,

    /**< System call interrupted (perhaps due to signal) */
    USB_ERROR_INTERRUPTED = -10,

    /**< Insufficient memory */
    USB_ERROR_NO_MEM = -11,

    /**< Operation not supported or unimplemented on this platform */
    USB_ERROR_NOT_SUPPORTED = -12,

    /* NB: Remember to update LIBUSB_ERROR_COUNT below as well as the
       message strings in strerror.c when adding new error codes here. */

    /**< Other error */
    USB_ERROR_OTHER = -99,
};

/**!
  * @enum FRAMETYPE
  * @brief The list of the image format which ncc camera supported.
  */
typedef enum
{
     YUV420p = 2,   //! < planar YUV4:2:0 format
     YUV422p,   //! < planar YUV422 8 bit
     H26X = 22, //! < H.264
     JPEG, //! < MJPEG
     METEDATA, //! < AI Meta data
     BLOB = 27, //! < AI model file for openvino
     BLOB_CFG, //! < AI modle config file
     NONE //! < reserve
}FRAMETYPE;

/**!
  * @enum METATYPE
  * @brief The list of the AI meta format which ncc camera supported.
  */
typedef enum
{
    META_FORMAT_U8=100, //! < U8
    META_FORMAT_FP16, //! < FP16
    META_FORMAT_FP32, //! < FP32
}METATYPE;

/**!
  * @enum PROCESS_MODE
  * @brief The list of the AI inference mode which ncc camera supported.
  */
typedef enum
{
    NCC_SYNC,
    NCC_ASYNC
}PROCESS_MODE;

/**!
  * @struct  NccUsbPortSpec_t
  * @brief USB port information to query device serial number
  */
typedef struct
{
    unsigned char length;  /** < Effective data length */
    unsigned char path[8]; /** < Port number(Such as Port_3.1->[3,1]) */
}NccUsbPortSpec_t;

/**!
  * @struct  NccPipeInput_t
  * @brief Frame data send to ncc device for AI inference
  */
typedef struct
{
    FRAMETYPE      type;            /** < Image format */
    unsigned int   seqNo;           /** < the frame sequence number */
    int            size;            /** < Bytes size of the frame */
    unsigned int   reserved[11];    /** < reserve */
    char          *input;  /** < Point to the memory of the frame data want to download to the Ncc device,\n
                                memory size should same size with NccPipeInput_t.size */
}NccPipeInput_t;

/**!
  * @struct  NccPipeOutput_t
  * @brief AI Meta frame with header and data, get back from ncc device after finished one frame AI inference
  */
typedef struct
{
    METATYPE       type;            /** <  AI Meta data format type */
    unsigned int   seqNo;           /** < frame sequence number updated by ncc device,help to know \n
                                          which video frame(struct NccPipeInput_t.seqNo) the current result corresponds to */
    int            actual_size;     /** < The actual AI meta data size of current frame */
    int            alloc_size;      /** < Size of your pre-allocated buffer */
    float          elapsed_time;    /** < Inference consumes time,millisecond */
    unsigned int   reserved[9];     /** < reserve */
    void          *output; /** < Point to the memory of the AI meta data for Ncc device to save the results,\n
                                The SDK is only responsible for using the cache, and the allocation and release need to be managed by the user.\n
                                You need to configure a large enough cache, generally according to your model output structure, \n
                                and the number of targets expected to support detection.When the cache required by the device exceeds\n
                                the size allocated by the user, the device only copies the maximum cache allocated by the user to feed back \n
                                the output data, which will lead to the loss of some reasoning results. */
}NccPipeOutput_t;

/**!
  * @struct  NccPipeHandle_t
  * @brief Inference engine handle. After the user initializes the model file information, \n
  * the SDK will automatically allocate inference engine resources according to the device conditions\n
  * to complete registration and initialization.
  * @code
  * NccPipeHandle_t handle = { NULL,\
                                 "auto",\
                                 "face-detection-retail-0004",\
                                 "/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/face-detection-retail-0004.blob",\
                                 "/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/config/input_BGR.json"};
    ret=ncc_pipe_create(&handle, NCC_SYNC);
    if(ret>0)
    {
        printf("ncc_pipe_create error %d !\n", ret);
        return -1;
    }
  * @endcode
  */
typedef struct
{
    void *parent;  /**  Parent handle to the engine，read-only   */
    char dev_sn[64];   /** <  Assigned engine's serial number */
    char pipe_name[128]; /** <  User named pipe name.*/
    char model_path[128]; /** < AI model file path to download and inference*/
    char json_path[128]; /** < The path of AI model parameter configuration file for inference needs to be downloaded   */
}NccPipeHandle_t;

/**!
  * @struct  NccTensorSpec_t
  * @brief Tensor structure
  */
typedef struct{
    int dim;       //! < Dim of vector
    int tensor[4]; /** <    tensor[B,C,H,W]
                     *      B - batch size
                     *      C - number of channels
                     *      H - image height
                     *      W - image width
                     */
}NccTensorSpec_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @fn sync_process
 * @brief Synchronous inference function,blocking operation. \n
 * return when the device inference is completed.
 * @param[in] *handle,Pointer to the inference handle,which has been initialized by SDK.
 * @param[in] *input, Pointer to the NccPipeInput_t
 * @param[in_out] *output, Pointer NccPipeOutput_t
 * @param[in] timeout_ms, time out millisecond. The sdk would return after timeout
 * @return Successful acquisition return or failure
 * @retval 0 success \n
 *        -1 timeout \n
 *        -2 repeated call
 * @code
    ....................
    NccPipeInput_t *pInData = ncc_malloc(imageWidth*imageHeight*3);
    if(pInData==0)
    {
        printf("ncc_malloc error\n");
        return 0;
    }
    ...........
    ...........
    NccPipeOutput_t pOutData;
    int maxOutSize = 1024*1024;

    pOutData.output = malloc(maxOutSize);
    if(pOutData.output==0)
    {
      printf("'nccMalloc error\n");
      return 0;
    }

    pOutData.alloc_size = maxOutSize;
    ......................
    ret = sync_process(&handle, pInData, &pOutData, 10000);
 * @endcode
 */
int sync_process(NccPipeHandle_t *handle, NccPipeInput_t *input, NccPipeOutput_t *output, unsigned int timeout_ms);

/**
 * @fn async_process
 * @brief Inference function,Asynchronous non blocking operation.\n
 * When the device inference is completed, the callback is triggered.
 * @param *handle, Pointer to the inference handle,which has been initialized by SDK.
 * @param *input, Pointer NccPipeInput_t
 * @return Successful acquisition return or failure
 * @retval 0  Success
 * @code
    ....................
    NccPipeInput_t *pInData = ncc_malloc(imageWidth*imageHeight*3);
    if(pInData==0)
    {
        printf("ncc_malloc error\n");
        return 0;
    }
    ...........
    ...........
    NccPipeOutput_t pOutData;
    int maxOutSize = 1024*1024;

    pOutData.output = malloc(maxOutSize);
    if(pOutData.output==0)
    {
      printf("'nccMalloc error\n");
      return 0;
    }

    pOutData.alloc_size = maxOutSize;
    ......................
    async_process(&handle, pInData, &pOutData, 10000);

    ncc_pipe_queue_read(handle, &pOutData, 0);
 * @endcode
 *
 */
int async_process(NccPipeHandle_t *handle, NccPipeInput_t *input);

/**!
 * @fn ncc_pipe_output_read
 * @brief Gets the output tensor information of the current pipeline
 * @param[in] *handle,pointer to the NccPipeHandle_t
 * @param[in] *pbuf, pointer to buffer
 * @param[in] time_out, option to control blocking read.
 * @return Successful acquisition return or failure
 * @retval 0 Success\n
 *        -1 Read error
 */
int ncc_pipe_queue_read(NccPipeHandle_t *handle, NccPipeOutput_t *pbuf, int time_out);

/**!
 * @fn ncc_malloc
 * @brief ncc sdk malloc memory block
 * @param[in] size, memory byte size want to malloc
 * @return Pointer to the NccPipeInput_t memory block
 * @retval 0 failed
 */
NccPipeInput_t *ncc_malloc(int size);

/**!
 * @fn ncc_free
 * @brief ncc sdk free memory block
 * @param[in] *pData, pointer to the NccPipeInput_t memory block
 * @return none
 */
void ncc_free(NccPipeInput_t *buff);

/**
 * @fn ncc_get_dev_number
 * @brief scan all the ncc devices connected with the Host
 * @return Number of devices scanned
 * @retval 0 non ncc devices scanned,others scanned number
 */
int ncc_dev_number_get(void);

/**
 * @fn ncc_dev_init
 * @brief Initialize ncc device,if your device don't have flash on it,need download the firmware
 * @param[in] *fw_path, file path of firmware
 * @param[in] dev_num, number of devices scanned
 * @return return init state
 * @retval >0 Number of devices successfully initialized\n
 *     -1 The input parameter exceeds the maximum number of devices \n
 *     -2 There are no devices need to initialize \n
 *     -3 USB initialization failed
 */
int ncc_dev_init(char *fw_path, int dev_num);

/**!
 * @fn ncc_dev_id_get
 * @brief Get index of device handle according to handle
 * @param[in] *handle, pointer to the NccPipeHandle_t
 * @return
 * @retval >0 index of device which handle allocated to\n
 *         -1 None corresponding handle created
 */
int ncc_dev_id_get(NccPipeHandle_t *handle);

/**!
 * @fn ncc_dev_serial_number_get
 * @brief Get serial number of device according to usb port
 * @param[in] *port, pointer to NccUsbPortSpec_t
 * @param[in] *string buffer to storage serial number
 * @param[in] *size size of string buffer
 * @return
 * @retval  0 Get successfully\n
 *         -1 No corresponding serial number found
 *         -2 Size of buffer is shorter than length of serial number
 */
int ncc_dev_serial_number_get(NccUsbPortSpec_t *port, char *string, int size);

/**!
 * @fn ncc_pipe_create
 * @brief Initialize the inference engine, read the parameters from the JSON file and \n
 * @param[in] *handle,pointer to the NccPipeHandle_t
 * @param[in] mode, enum PROCESS_MODE
 * @return Successful acquisition return or failure
 * @par Sync blocking mode demo：
 * @code
 * NccPipeHandle_t handle = { NULL,\
                                 "auto",\
                                 "face-detection-retail-0004",\
                                 "/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/face-detection-retail-0004.blob",\
                                 "/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/config/input_BGR.json"};
    ret=ncc_pipe_create(&handle, NCC_SYNC);
    if(ret>0)
    {
        printf("ncc_pipe_create error %d !\n", ret);
        return -1;
    }
    ...
    ...
 * @codeend
 */
int ncc_pipe_create(NccPipeHandle_t *handle, PROCESS_MODE mode);

/**!
 * @fn ncc_dev_start
 * @brief Start all the pipelines on the device with specified serial number
 * @param[in] dev_id, serial number of the device
 * @return Successful acquisition return or failure
 * @retval 0 Start successfully\n
 *        -1 Start failed
 */
int ncc_dev_start(int dev_id);

/**!
 * @fn ncc_pipe_id_get
 * @brief Get the pipe index of this NccPipeHandle_t on the device.\n
 * Generally speaking, this pipe index is automatically assigned by SDK.
 * @param[in] *handle, pointer to the NccPipeHandle_t
 * @return index of pipe
 */
int ncc_pipe_id_get(NccPipeHandle_t *handle);

/**!
 * @fn ncc_input_tensor_descriptor_get
 * @brief Gets the input tensor information of the current pipeline
 * @param[in] *handle,pointer to the NccPipeHandle_t
 * @param[in] *input_tensor,pointer to the NccTensorSpec_t
 * @return 0 Success\n
 *        -1 Failure
 */
int ncc_input_tensor_descriptor_get(NccPipeHandle_t *handle, NccTensorSpec_t *input_tensor);

/**!
 * @fn ncc_output_tensor_descriptor_get
 * @brief Gets the output tensor information of the current pipeline
 * @param[in] *handle, pointer to the NccPipeHandle_t
 * @param[in] *output_tensor, pointer to the NccTensorSpec_t
 * @return 0 Success\n
 *        -1 Failure
 */
int ncc_output_tensor_descriptor_get(NccPipeHandle_t *handle, NccTensorSpec_t *output_tensor);

#ifdef __cplusplus
}
#endif
#endif
