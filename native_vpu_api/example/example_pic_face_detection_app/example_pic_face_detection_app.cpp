#include <includes.h>
#include <cameraCtrl.h>
#include <native_vpu_api.h>

#include <linux/videodev2.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>

#include "native_vpu_api.h"

using namespace cv;

typedef struct
{
    float image_id;
    float label;
    float conf;
    float x_min;
    float y_min;
    float x_max;
    float y_max;
}OvDetectSpec_t;

cv::Mat showImage[MAX_DEV_NUM*MAX_PIPELINE_NUM];
char showname[MAX_DEV_NUM*MAX_PIPELINE_NUM][100];
int  showStart[MAX_DEV_NUM*MAX_PIPELINE_NUM]={0};

cv::Mat cv_image_import(char *filepath, int *imageWidth, int *imageHeight);
void cv_tensor_convert(NccPipeHandle_t *handle, cv::Mat src, char* output, int width, int height);

void obj_show_img_func(char *data, int w, int h, float scale, char *name, float *pMeta,float min_score, int id);
void showAllVideo(void);

int main(int argc, char* argv[])
{
    if(argc <2)
    {
        printf("Please input sudo ./example_pic_face_detection_app [<image(*.jpeg)>]");
        return 0;
    }

    int ret = 0;
    int testseq = 0;

    /* 1.detect device */
    int devNum = ncc_dev_number_get();
    printf("nccLinkInit find devs %d\n", devNum);
    if(devNum<=0)
    {
        printf("ncc_dev_number_get error, the device was not found\n");
        return -1;
    }

    /* 2.initialize device, load firmware */
    ret = ncc_dev_init("/usr/local/lib/openncc/OpenNcc.mvcmd", devNum);
    if(ret<0)
    {
        printf("ncc_dev_init error\n");
        return -1;
    }
    else
    {
        printf("ncc_dev_init success num %d\n",ret);
    }

    NccPipeHandle_t handle = { NULL,\
                                 "",\
                                 "face-detection-retail-0004",\
                                 "/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/face-detection-retail-0004.blob",\
                                 "/usr/local/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/config/input_BGR.json"};

    /* 3.create handle, synchronous mode */
    ret=ncc_pipe_create(&handle, NCC_SYNC);
    if(ret>0)
    {
        printf("ncc_pipe_create error %d !\n", ret);
        return -1;
    }

    /* 4.start device */
    for(int i=0;i<devNum;i++)
    {
        ret = ncc_dev_start(i);
        if(ret>0)
        {
           printf("ncc_dev_start error %d !\n", ret);
           return -1;
        }
    }

    int imageWidth;
    int imageHeight;

    while(1)
    {
        /* import image */
        cv::Mat cv_img = cv_image_import(argv[1], &imageWidth, &imageHeight);
        if(cv_img.empty())
        {
            return -1;
        }

        /* allocates memory for storing images sent to the device */
        NccPipeInput_t *pInData = ncc_malloc(imageWidth*imageHeight*3);
        if(pInData==0)
        {
            printf("ncc_malloc error\n");
            return 0;
        }

        pInData->seqNo = testseq++;

        /* get input size of model */
        NccTensorSpec_t input;
        ncc_input_tensor_descriptor_get(&handle, &input);
        int inputDimWidth  = input.tensor[3];
        int inputDimHeight = input.tensor[2];

        /* convert BGR_i --> BGR_P */
        cv_tensor_convert(&handle, cv_img, (char *)pInData->input, inputDimWidth, inputDimHeight);
        pInData->size = inputDimWidth*inputDimHeight*3;

        /* allocates memory for storing meta data */
        NccPipeOutput_t pOutData;
        int maxOutSize = 1024*1024;

        pOutData.output = malloc(maxOutSize);
        if(pOutData.output==0)
        {
          printf("'malloc error\n");
          return 0;
        }

        pOutData.alloc_size = maxOutSize;

        /* Invoke the synchronous inference interface，overtime is 10s */
        ret = sync_process(&handle, pInData, &pOutData, 10000);
        printf("%s, inSeq:%d outSeq:%d outSize:%d ,T:%d MS, pid:%d\n",handle.pipe_name, pInData->seqNo ,\
                pOutData.seqNo, pOutData.actual_size, (int)pOutData.elapsed_time, ncc_pipe_id_get(&handle));

        /* display */
        char str[128];
        float scale = 1.0*960/imageWidth;
        float min_score=0.5;
        sprintf(str, "%s",handle.pipe_name);
        obj_show_img_func((char*)cv_img.data, imageWidth, imageHeight, scale, str, (float*)pOutData.output, min_score, ncc_pipe_id_get(&handle));

        showAllVideo();

        /* free memory */
        ncc_free(pInData);
        free(pOutData.output);

        pOutData.output = 0;
        pInData = 0;
    }
}

cv::Mat cv_image_import(char *filepath, int *imageWidth, int *imageHeight)
{
    cv::Mat cv_img = cv::imread(filepath, IMREAD_COLOR);
    if (cv_img.empty())
    {
        fprintf(stderr, "cv::imread %s failed\n", filepath);
        return cv_img;
    }

    *imageWidth  = cv_img.cols;
    *imageHeight = cv_img.rows;

    return cv_img;
}

void cv_tensor_convert(NccPipeHandle_t *handle, cv::Mat src, char* output, int width, int height)
{
    cv::Mat dest;

    /* scale to input size according to model */
    resize(src, dest, Size(width, height), 0, 0, INTER_LINEAR);

    /* BGR_i-->BGR_P */
    char *p_b  = output;
    char *p_g  = output + width*height;
    char *p_r  = output + width*height*2;
    char *p_in = (char *)(dest.data);

    for(int i=0;i<width*height;i++)
    {
        p_b[i] = p_in[3*i+0];
        p_g[i] = p_in[3*i+1];
        p_r[i] = p_in[3*i+2];
    }
}

int coordinate_is_valid(float x1, float y1, float x2, float y2)
{
    if((x1<0) || (x1>1))
        return 0;
    if((y1<0) || (y1>1))
        return 0;
    if((x2<0) || (x2>1))
        return 0;
    if((y2<0) || (y2>1))
        return 0;
    if((x1>=x2) || (y1>=y2))
        return 0;

    return 1;
}

void obj_show_img_func(char *img, int w, int h, float scale, char *name, float *pMeta,float min_score, int id)
{
    cv::Mat img_out;
    img_out.create(h, w, CV_8UC3);
    int i,dis_w,dis_h,oft_x, oft_y;

    img_out.data = (unsigned char*)img;

    /* get fov */
    oft_x = 0;
    oft_y = 0;
    dis_w = w;
    dis_h = h;

    for (i = 0; i < 200; i++)
    {
        OvDetectSpec_t box;
        memcpy(&box, &pMeta[i*7], sizeof(OvDetectSpec_t));

        /* eliminate invalid data and low score */
        if(  (coordinate_is_valid(box.x_min,box.y_min,box.x_max,box.y_max) ==0 )
                ||(box.conf < min_score)
                || box.image_id<0
                )
        {
            continue;
        }

        /* draw box */
        cv::Rect object;
        object.x = box.x_min * dis_w + oft_x;
        object.y = box.y_min * dis_h + oft_y;
        object.width  = (box.x_max - box.x_min) * dis_w;
        object.height = (box.y_max - box.y_min) * dis_h;

//      printf("box %d %d %d %d,dis_w(%d %d)\n", object.x,object.y, object.x +object.width,object.y+object.height, dis_w,dis_h);
        cv::rectangle(img_out, object, cv::Scalar(255, 128, 128), 2, 8, 0);

        cv::Point origin;
        origin.x = object.x ;
        origin.y = object.y ;
        char   result[128];
        memset(result, 0, sizeof(result));
        sprintf(result, "score:%d%%", (int)(100*box.conf));
        cv::putText(img_out, result, origin, cv::FONT_HERSHEY_COMPLEX, 0.5,  cv::Scalar(255, 255, 128), 1, 8, 0);
    }

    /* scale */
    resize(img_out,showImage[id], Size(img_out.cols*scale,img_out.rows*scale),0,0,INTER_LINEAR);
    strcpy(showname[id], name);
    showStart[id] = 1;
}

void showAllVideo(void)
{
    for(int i=0;i<MAX_DEV_NUM*MAX_PIPELINE_NUM;i++)
    {
        if(showStart[i])
            cv::imshow(showname[i], showImage[i]);
    }
    cv::waitKey(10);
}
