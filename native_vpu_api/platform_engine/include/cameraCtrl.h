/**
 * @file   cameraCtrl.h
 * @author Zed
 * @date   2021.11.15
 * @brief Control and configure general parameters of NCC camera
 */

#ifndef _CAMERA_CONTROL_H
#define _CAMERA_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @define MAX_MODE_SIZE
 * @brief The maximum of the modes which the ncc cameras supported
*/
#define MAX_MODE_SIZE   5

/**
  * @enum CAM_CTRL_AWB_MODE
  * @brief The WB modes list,could disable or enable the AWB,swithc to MWB mode.
  */
typedef enum {
    CAMERA_CONTROL_AWB_MODE__OFF, //! < Disable the WB mode,and switch to MWB mode,user could set the WB mode,0
    CAMERA_CONTROL_AWB_MODE__AUTO, //! < Enable the WB mode,1
    CAMERA_CONTROL_AWB_MODE__INCANDESCENT, //! < MWB for INCANDESCENT,2
    CAMERA_CONTROL_AWB_MODE__FLUORESCENT, //! < MWB for FLUORESCENT,3
    CAMERA_CONTROL_AWB_MODE__WARM_FLUORESCENT, //! < MWB for WARM_FLUORESCENT,4
    CAMERA_CONTROL_AWB_MODE__DAYLIGHT, //! < MWB for DAYLIGHT,5
    CAMERA_CONTROL_AWB_MODE__CLOUDY_DAYLIGHT, //!< MWB for CLOUDY_DAYLIGHT,6
    CAMERA_CONTROL_AWB_MODE__TWILIGHT, //! < for MWB for TWILIGHT,7
    CAMERA_CONTROL_AWB_MODE__SHADE, //! < MWB for SHADE,8
}CAM_CTRL_AWB_MODE;

/**
  * @enum VIDEO_CTRL_OUT_MDOE
  * @brief Set the video stream mode of the ncc camera
  */
typedef enum
{
    VIDEO_OUT_DISABLE,       /*! <  disable video out */
    VIDEO_OUT_SINGLE,        /*! < output one frame,could used to capture frame */
    VIDEO_OUT_CONTINUOUS,    /*! < output continues frames as the frame rates setted */
}VIDEO_CTRL_OUT_MDOE;

/**
  * @struct SensorModesPara_t
  * @brief List of camera control parameters
  */
typedef struct
{
    char moduleName[16]; //! < the name of the sensor
    int  camWidth;   //! < output resolution of the width
    int  camHeight;  //! < output resolution of the height
    int  camFps;     //! < frame rates per second
    int  AFmode;     //! < support AF or not, if your camera use fixed focal lens, not support it
    int  maxEXP;     //! < maximum exposure time, unit us
    int  minGain;    //! < min exposure gain value,100 means x1
    int  maxGain;    //! < max exposure gain value,1600
} SensorModesPara_t;

/**
  * @struct SensorModesList_t
  * @brief List of camera control parameters
  */
typedef struct
{
    int               num;  //! < the active number of the sensor modes supported by the camera,get from camera
    SensorModesPara_t mode[MAX_MODE_SIZE]; //! < the items of the sensor modes
} SensorModesList_t;

/**
 * @brief Get index of device handle according to serial number
 * @param[in] *serial_num serial number of device
 * @return
 *   @retval >0 index of device controller corresponding to serial number\n
 *   @retval -1 No corresponding serial number found
 */
int device_ctrl_id_get(char* serial_num);

/**
 * @brief Get mode list which camera supported
 * @param[in] dev_index, the active ncc device scaned by ncc sdk,could use the device's index to config camera
 * @param[out] *list, point to a SensorModesList_t
 * @return Successful acquisition return or failure
 * @retval 0  success, others follow enum usb_error
 * @par Before call this function,you need call ncc_get_dev_number(void) to scan the ncc devices
 * @code
 *  int devNum = ncc_get_dev_number();
 *  if(devNum<=0)
 *      return -1;
 *  ..if your ncc device don't have flash on it...
 *  ret = nccDevInit("/usr/local/lib/openncc/OpenNcc.mvcmd", devNum);
 *  ....
 *  SensorModesList_t list;
 *  int num = device_ctrl_get_sensor_mode_list(0, &list);
 * @endcode
 * @see native_vpu_api.h ncc_get_dev_number(void)\n
 * native_vpu_api.h  enum usb_error
 */
int device_ctrl_get_sensor_mode_list(int dev_index, SensorModesList_t* list);

/**
 * @brief Select sensor and resolution
 * @param[in] dev_index [Range from 0 to SensorModesConfig.num]
 * @param[in] mode_index ,selecte the SensorModesPara_t index get from camera
 * @return Successful acquisition return or failure
 * @retval 0  success, others follow enum usb_error
 * @par After you selected the mode,the camera would work as the SensorModesPara listed, \n
 * could call it after device_ctrl_get_sensor_mode_list(...)
 * @bug Currently only support 1080P
 */
int device_ctrl_select_sensor_mode(int dev_index, int mode_index);


/**
 * @brief Get firmware version
 * @param[in] dev_index, the active ncc device scaned by ncc sdk,could use the device's index to config camera
 * @param[out] *fw_version, get the firmware version from ncc camera or device
 * @return Successful acquisition return or failure
 * @retval 0  success, others follow enum usb_error
 * @par null
 * @code
 *  int devNum = ncc_get_dev_number();
 *  if(devNum<=0)
 *      return -1;
 *  ..if your ncc device don't have flash on it,need download firmware to the device...
 *  ret = nccDevInit("/usr/local/lib/openncc/OpenNcc.mvcmd", devNum);
 *  ....
 *  char version[100];
 *  ret = device_ctrl_get_fw_version(0, version);
 * @endcode
 * @see native_vpu_api.h ncc_get_dev_number(void) \n
 * native_vpu_api.h nccDevInit(...)
 */
int device_ctrl_get_fw_version(int dev_index, char* fw_version, int size);

/**
 * @brief Enable AE of the camera.
 * @param[in] dev_index [Range from 0 to SensorModesConfig.num]
 * @return Successful acquisition return or failure
 * @retval 0  success, others follow enum usb_error
 * @note After call camera_ctrl_me_set_exp_gain(...),the camera would automatic enter ME mode.We need call camera_ctrl_ae_enabled
 * switch back to AE mode.
 */
int camera_ctrl_ae_enabled(int dev_index);


/**
 * @brief Set the exposure time and gain of the camera
 * @param[in] dev_index, if detected more than two cameras ,need selete one [Range from 0 to SensorModesConfig.num]
 * @param[in] exp_us,[unit: us, Max value: 1/fps*1000*1000]
 * @param[in] iso_val,exposure gain.The range of the value is get from SensorModesPara_t.minGain and SensorModesPara_t.maxGain
 * @return Successful acquisition return or failure
 * @retval 0  success, others follow enum usb_error
 * @par You could get the frame rate from SensorModesPara_t.camFps
 */
int camera_ctrl_me_set_exp_gain(int dev_index, unsigned int exp_us, unsigned int iso_val);

/**
 * @brief Set white balance mode
 * @param[in] dev_index, if detected more than two cameras ,need selete one [Range from 0 to SensorModesConfig.num]
 * @param[in] awb_mode,CAM_CTRL_AWB_MODE mode
 * @return Successful acquisition return or failure
 * @retval 0  success, others follow enum usb_error
 */
int camera_ctrl_set_awb_mode(int dev_index, CAM_CTRL_AWB_MODE awb_mode);

#ifdef __cplusplus
}
#endif
#endif /* _CAMERA_CONTROL_H */
