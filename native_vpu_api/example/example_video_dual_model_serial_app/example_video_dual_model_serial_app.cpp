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
    void *start;
    size_t length;
} VideoSpec_t;

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

int  v4l2_device_open(char *node);
int  v4l2_device_start(int fd);
int  v4l2_device_fill(int fd, struct v4l2_buffer *src);
int  v4l2_device_queue(int fd, struct v4l2_buffer *src);
void v4l2_device_query(int fd, int *imageWidth, int *imageHeight);
VideoSpec_t* v4l2_device_allocate(int fd, struct v4l2_buffer *src);

void cv_tensor_convert(cv::Mat src, char* output, int width, int height);

void FDlandMarks(char *img, int w, int h, float scale, char *name, float *pMeta, float *Marks, float min_score, int id);
int  coordinate_is_valid(float x1, float y1, float x2, float y2);
void showAllVideo(void);

int main(int argc, char* argv[])
{
    if(argc <2)
    {
        printf("Please input sudo ./example_video_control_app [<node(/dev/video*)>]");
        return 0;
    }

    int ret = 0;
    int testseq = 0;

    /* 1.detect device */
    int devNum = ncc_dev_number_get();
    printf("ncc_dev_number_get find devs %d\n", devNum);
    if(devNum<=0)
    {
        printf("nccLinkInit error, the device was not found\n");
        return -1;
    }

    /* 2.initialize device, load firmware */
    ret = ncc_dev_init("/usr/lib/openncc/OpenNcc.mvcmd", devNum);
    if(ret<0)
    {
        printf("ncc_dev_init error\n");
        return -1;
    }
    else
    {
        printf("ncc_dev_init success num %d\n",ret);
    }

    NccPipeHandle_t handleFD = { NULL,\
                                 "",\
                                 "face-detection-retail-0004",\
                                 "/usr/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/face-detection-retail-0004.blob",\
                                 "/usr/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/config/input_yuv420_1080P.json"};

    NccPipeHandle_t handleLM = {NULL,\
                                 "",\
                                 "landmarks-regression-retail-0009",\
                                 "/usr/lib/openncc/model_zoo/ncc/openvino_2021.4/landmarks-regression-retail-0009/landmarks-regression-retail-0009.blob",\
                                 "/usr/lib/openncc/model_zoo/ncc/openvino_2021.4/landmarks-regression-retail-0009/config/input_BGR.json"};

    /* 3.create handle, synchronous mode */
    ret = ncc_pipe_create(&handleFD, NCC_SYNC);
    if(ret>0)
    {
        printf("ncc_pipe_create error %d !\n", ret);
        return -1;
    }

    ret = ncc_pipe_create(&handleLM, NCC_SYNC);
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

    int imageWidth,imageHeight;

    /* 5. open UVC camera*/
    int fd;
    fd = v4l2_device_open(argv[1]);

    /* query camera video format */
    v4l2_device_query(fd, &imageWidth, &imageHeight);

    /* 6. memory allocate */
    struct v4l2_buffer src;
    VideoSpec_t *frame = v4l2_device_allocate(fd, &src);
    if(frame == 0)
        exit(0);

    /* 7. start capturing images */
    ret = v4l2_device_start(fd);
    if(ret == -1)
    {
        perror("VIDIOC_STREAMON failed!\n");
    }

    while(1)
    {
        /* 1. read video frame */
        ret = v4l2_device_fill(fd, &src);
        if (ret == -1)
        {
            perror("VIDIOC_DQBUF failed!\n");
            usleep(10000);
            continue;
        }

        /* get frame index and size */
        unsigned int *pSeqNo = (unsigned int *)frame[src.index].start;
        unsigned int size = imageWidth*imageHeight*3/2;

        /* 2. allocates memory for storing images sent to the device */
        NccPipeInput_t *pInData = ncc_malloc(size);
        if(pInData==0)
        {
            printf("ncc_malloc error\n");
            return 0;
        }
        float min_score=0.5;
        pInData->seqNo = pSeqNo[0];

        cv::Mat img_yuv;
        img_yuv.create(imageHeight * 3 / 2, imageWidth, CV_8UC1);
        img_yuv.data = (unsigned char*)frame[src.index].start;

        memcpy(pInData->input, (char*)img_yuv.data, imageWidth*imageHeight*3/2);

        /* 3. allocates memory for storing meta data */
        NccPipeOutput_t pOutData;
        int maxOutSize = 1024*1024;

        pOutData.output = malloc(maxOutSize);
        if(pOutData.output==0)
        {
          printf("'ncc_malloc error\n");
          return 0;
        }
        pOutData.alloc_size = maxOutSize;

        /* 4. Invoke the synchronous inference interface，overtime is 10s */
        ret = sync_process(&handleFD, pInData, &pOutData, 10000);
//        printf("%s, inSeq:%d outSeq:%d outSize:%d ,T:%d MS, pid:%d\n",handleFD.pipelineName, pInData->seqNo ,\
//                pOutData.seqNo, pOutData.size, (int)pOutData.procTimeMS, getPipelineID(&handleFD));
        if(ret!=0)
        {
            printf("syncProc err, ret=%d\n", ret);
            /* free memory */
            ncc_free(pInData);
            free(pOutData.output);
            continue;
        }

        /*** second model inference ***/
        /* 1. get input size of model */
        NccTensorSpec_t input;
        ncc_input_tensor_descriptor_get(&handleLM, &input);
        int inputDimWidth =  input.tensor[3];
        int inputDimHeight = input.tensor[2];
        size = inputDimWidth*inputDimHeight*3;

        /* 2. allocates memory for storing images sent to the device */
        NccPipeInput_t *pInData2 = ncc_malloc(size);
        if(pInData==0)
        {
            printf("ncc_malloc error\n");
            return 0;
        }

        /* 3. allocates memory for storing meta data */
        NccPipeOutput_t pOutData2;
        pOutData2.output = malloc(maxOutSize);
        if(pOutData2.output==0)
        {
            printf("'ncc_malloc error\n");
            return 0;
        }
        pOutData2.alloc_size = maxOutSize;

        cv::Mat img_bgr;
        cv::cvtColor(img_yuv, img_bgr, CV_YUV2BGR_I420);

       /* eliminate invalid*/
        float *detMetadata = (float*)pOutData.output;
        float marks[20*20];
        for (int i = 0; i < 200; i++)
        {
            /*  format: [image_id, label, conf, x_min, y_min, x_max, y_max] */
            OvDetectSpec_t box;
            memcpy(&box, &detMetadata[i*7], sizeof(OvDetectSpec_t));

            /* 3. eliminate invalid data and low score */
            if(  (coordinate_is_valid(box.x_min,box.y_min,box.x_max,box.y_max) ==0 )
                    ||(box.conf < min_score)
                    || box.image_id<0
                    )
            {
                continue;
            }

            /* 4. Extracting face data and convert */
            cv::Mat face = img_bgr(Range((int)(imageHeight*box.y_min),(int)(imageHeight*box.y_max)),Range((int)(imageWidth*box.x_min),(int)(imageWidth*box.x_max)));
            cv_tensor_convert(face, pInData2->input, inputDimWidth, inputDimHeight);

            /* 5. Invoke the synchronous inference interface，overtime is 10s */
            ret = sync_process(&handleLM, pInData2, &pOutData2, 10000);
//          printf("%s, inSeq:%d outSeq:%d outSize:%d ,T:%d MS\n",handleLandMarks.pipelineName, pInData->seqNo ,\
//                    pOutData2.seqNo, pOutData2.size, (int)pOutData2.procTimeMS);

            /* 6. save the result of face eigenvalue */
            memcpy(&marks[10*i], pOutData2.output, 40);
        }

        /*** second model inference ***/

        /* 5. display */
        char str[128];
        float scale = 1.0*960/imageWidth;

        sprintf(str, "%s->%s",handleFD.pipe_name,handleLM.pipe_name);
        FDlandMarks((char *)img_yuv.data, imageWidth, imageHeight, scale, str, (float*)pOutData.output, marks, min_score, ncc_pipe_id_get(&handleFD));
        showAllVideo();

        /* 6. free memory*/
        ncc_free(pInData);
        ncc_free(pInData2);
        free(pOutData.output);
        free(pOutData2.output);

        pOutData.output = 0;
        pOutData2.output = 0;
        pInData = 0;

        /* 7. The frame buffer is re-queued into the input queue. */
        ret = v4l2_device_queue(fd, &src);
        if(ret == -1)
        {
            perror("VIDIOC_QBUF failed!\n");
            continue;
        }
    }
}

int v4l2_device_open(char *node)
{
    int fd;
    fd = open(node,O_RDWR);

    return fd;
}

void v4l2_device_query(int fd, int *imageWidth, int *imageHeight)
{
    struct v4l2_format fmt;
    fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_G_FMT, &fmt);

    *imageWidth = fmt.fmt.pix.width;
    *imageHeight = fmt.fmt.pix.height;
    printf("Current data format information:\n\twidth:%d\n\theight:%d\n",fmt.fmt.pix.width,fmt.fmt.pix.height);

    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("Supprotformat:\n");
    while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)!=-1)
    {
        printf("\t%d.%c%c%c%c\t%s\n",fmtdesc.index+1,fmtdesc.pixelformat & 0xFF,\
                (fmtdesc.pixelformat >> 8) & 0xFF,(fmtdesc.pixelformat >> 16) & 0xFF, (fmtdesc.pixelformat >> 24) & 0xFF,fmtdesc.description);
        fmtdesc.index++;
    }
}

VideoSpec_t* v4l2_device_allocate(int fd, struct v4l2_buffer *src)
{
    struct v4l2_requestbuffers reqbuf;
    reqbuf.count = 4;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;

    if(ioctl(fd, VIDIOC_REQBUFS, &reqbuf) == -1)
    {
        perror("VIDIOC_REQBUFS failed!\n");
        return (VideoSpec_t*)0;
    }

    //Map buffer to user program
    VideoSpec_t *frame;
    frame = (VideoSpec_t*)calloc(reqbuf.count,sizeof(VideoSpec_t));

    for(int i=0;i<reqbuf.count;i++)
    {
        src->index = i;
        src->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        src->memory = V4L2_MEMORY_MMAP;
        if (ioctl(fd, VIDIOC_QUERYBUF, src) == -1)
        {
            perror("VIDIOC_QUERYBUF failed!\n");
            return (VideoSpec_t*)0;
        }

        //map buffer
        frame[i].length = src->length;
        frame[i].start = mmap(NULL, src->length,
                                 PROT_READ | PROT_WRITE,
                                 MAP_SHARED, fd, src->m.offset);

        if (frame[i].start == MAP_FAILED)
        {
            perror("mmap failed!\n");
            return (VideoSpec_t*)0;
        }

        //buffer queue
        if (ioctl(fd, VIDIOC_QBUF, src) == -1)
        {
            perror("VIDIOC_QBUF failed!\n");
            return (VideoSpec_t*)0;
        }
    }

    return frame;
}

int v4l2_device_start(int fd)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret = ioctl(fd, VIDIOC_STREAMON, &type);

    return ret;
}

int v4l2_device_fill(int fd, struct v4l2_buffer *src)
{
    src->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    src->memory = V4L2_MEMORY_MMAP;
    int ret = ioctl(fd, VIDIOC_DQBUF, src);

    return ret;
}

int v4l2_device_queue(int fd, struct v4l2_buffer *src)
{
    int ret = ioctl(fd, VIDIOC_QBUF, src);
    return ret;
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

void cv_tensor_convert(cv::Mat src, char* output, int width, int height)
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

void FDlandMarks(char *img, int w, int h, float scale, char *name, float *pMeta, float *Marks, float min_score, int id)
{
    cv::Mat img_yuv;
    img_yuv.create(h * 3 / 2, w, CV_8UC1);
    cv::Mat img_out;
    int i,dis_w,dis_h,oft_x, oft_y;

    /* YUV420P-->RGB */
    img_yuv.data = (unsigned char*)img;
    cv::cvtColor(img_yuv, img_out, CV_YUV2BGR_I420);

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
        ////////////////////////////////////////////////////////////////////////////

        /* draw eigenvalue */
        float* regMetadata = &Marks[10*i];
        int markSize = 6;
        cv::Point p2;
        p2.x =  (regMetadata[0]) * object.width  + object.x ;
        p2.y =  (regMetadata[1]) * object.height  + object.y;
        cv::circle(img_out, p2, markSize, Scalar(0,0,255),-1);

        p2.x =  (regMetadata[2]) * object.width  + object.x ;
        p2.y =  (regMetadata[3]) * object.height  + object.y;
        cv::circle(img_out, p2, markSize, Scalar(0,0,255),-1);

        p2.x =  (regMetadata[4]) * object.width  + object.x ;
        p2.y =  (regMetadata[5]) * object.height  + object.y;
        cv::circle(img_out, p2, markSize, Scalar(0,0,255),-1);

        p2.x =  (regMetadata[6]) * object.width  + object.x ;
        p2.y =  (regMetadata[7]) * object.height  + object.y;
        cv::circle(img_out, p2, markSize, Scalar(0,0,255),-1);

        p2.x =  (regMetadata[8]) * object.width  + object.x ;
        p2.y =  (regMetadata[9]) * object.height  + object.y;
        cv::circle(img_out, p2, markSize, Scalar(0,0,255),-1);
////////////////////////////////////////////////////////////////////////////

        cv::Point origin;
        origin.x = object.x ;
        origin.y = object.y + 32;
        char   result[128];
        memset(result, 0, sizeof(result));
        sprintf(result, "%d_score:%d%%", (int)box.label, (int)(100*box.conf));
        cv::putText(img_out, result, origin, cv::FONT_HERSHEY_COMPLEX, 1,  cv::Scalar(255, 255, 128), 1, 8, 0);
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
