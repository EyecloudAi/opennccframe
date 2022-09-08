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
    METATYPE       type;
    unsigned int   seqNo;
    int            size;
    float          procTimeMS;
    char metaData[5*1024*1024];
}asyncBuffSpev_t;

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
sem_t videoDone;

static pthread_mutex_t procMutex;
static char dev_node[32];
static char image_path[128];

int  v4l2_device_open(char *node);
int  v4l2_device_start(int fd);
int  v4l2_device_fill(int fd, struct v4l2_buffer *src);
int  v4l2_device_queue(int fd, struct v4l2_buffer *src);
void v4l2_device_query(int fd, int *imageWidth, int *imageHeight);
VideoSpec_t* v4l2_device_allocate(int fd, struct v4l2_buffer *src);

cv::Mat cv_image_import(char *filepath, int *imageWidth, int *imageHeight);

void obj_show_img_func(char *data, int w, int h, float scale, char *name, float *pMeta,float min_score, int id);
void showAllVideo(void);

static void *FDSrcUvcCameraThread(void* arg)
{
    NccPipeHandle_t *handle = (NccPipeHandle_t *)arg;
    int ret = 0;
    int imageWidth, imageHeight;
    int pipe_id = ncc_pipe_id_get(handle);

    /* 1. open UVC camera*/
    int fd;
    fd = v4l2_device_open(dev_node);

    /* query camera video format */
    v4l2_device_query(fd, &imageWidth, &imageHeight);

    /* 2. memory allocate */
    struct v4l2_buffer src;
    VideoSpec_t *frame = v4l2_device_allocate(fd, &src);
    if(frame == 0)
        exit(0);

    /* 3. start capturing images */
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

        /* 2. allocates memory for storing meta data */
        NccPipeOutput_t pOutData;
        int maxOutSize = 1024*1024;

        pOutData.output = malloc(maxOutSize);
        if(pOutData.output==0)
        {
          printf("'nccMalloc error\n");
          return 0;
        }
        pOutData.alloc_size = maxOutSize;

        /* 3. read meta data */
        ncc_pipe_queue_read(handle, &pOutData, 0);

        /* 4. display */
        char str[128];
        float scale = 1.0*960/imageWidth;
        float min_score=0.8;
        sprintf(str, "%s",handle->pipe_name);

        obj_show_img_func((char*)frame[src.index].start, imageWidth, imageHeight, scale, str, (float*)pOutData.output, min_score, pipe_id);

        /* 5. free memory*/
        pOutData.output = 0;

        /* 6. The frame buffer is re-queued into the input queue. */
        ret = v4l2_device_queue(fd, &src);
        if(ret == -1)
        {
            perror("VIDIOC_QBUF failed!\n");
            continue;
        }

        sem_post(&videoDone);
    }
}

static void *PDSrcDevCameraThread(void* arg)
{
    NccPipeHandle_t *handle = (NccPipeHandle_t *)arg;
    int ret = 0;
    int imageWidth,imageHeight;
    int testseq = 0;
    int pipe_id = ncc_pipe_id_get(handle);

    /* import image */
    cv::Mat cv_img = cv_image_import(image_path, &imageWidth, &imageHeight);
    if(cv_img.empty())
    {
        exit(-1);
    }

    int size = imageWidth*imageHeight*3/2;

    cv::Mat cv_yuv;
    cv::cvtColor(cv_img, cv_yuv, CV_BGR2YUV_I420);

    while(1)
    {
        /* allocates memory for storing images sent to the device */
        NccPipeInput_t *pInData = ncc_malloc(size);
        if(pInData==0)
        {
            printf("ncc_malloc error\n");
        }

        pInData->seqNo = testseq++;
        pInData->size  = size;

        memcpy(pInData->input, cv_yuv.data, size);

        /* Invoke the asynchronous inference interface, overtime is 10s */
        ret = async_process(handle, pInData);

        /* allocates memory for storing meta data */
        NccPipeOutput_t pOutData;
        int maxOutSize = 1024*1024;

        pOutData.output = malloc(maxOutSize);
        if(pOutData.output==0)
        {
          printf("'nccMalloc error\n");
          return 0;
        }
        pOutData.alloc_size = maxOutSize;

        /* read meta data */
        ncc_pipe_queue_read(handle, &pOutData, 0);

        /* display */
        char str[128];
        float scale = 1.0*960/imageWidth;
        float min_score=0.5;
        sprintf(str, "%s",handle->pipe_name);
        obj_show_img_func((char*)cv_yuv.data, imageWidth, imageHeight, scale, str, (float*)pOutData.output, min_score, pipe_id);

        /* free memory */
        ncc_free(pInData);
        pInData = 0;

        sem_post(&videoDone);
    }
}

int main(int argc, char* argv[])
{
    if(argc <3)
    {
        printf("Please input sudo ./example_video_dual_model_parallel_app [<node(/dev/video*)>] [<image(*.jpeg)>]\n");
        return 0;
    }

    strcpy(dev_node, argv[1]);
    strcpy(image_path, argv[2]);

    sem_init(&videoDone, 0, 0);
    pthread_mutex_init(&procMutex, NULL);

    int ret = 0;

    /* 1.detect device */
    int devNum = ncc_dev_number_get();
    printf("nccLinkInit find devs %d\n", devNum);
    if(devNum<=0)
    {
        printf("ncc_dev_number_get error, the device was not found\n");
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

    NccPipeHandle_t handleFD = { 0,\
                                 "",\
                                 "face-detection-retail-0004",\
                                 "/usr/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/face-detection-retail-0004.blob",\
                                 "/usr/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/config/input_camera.json"};

    NccPipeHandle_t handlePD = { 0,\
                                 "",\
                                 "person-detection-retail-0013",\
                                 "/usr/lib/openncc/model_zoo/ncc/openvino_2021.4/person-detection-retail-0013/person-detection-retail-0013.blob",\
                                 "/usr/lib/openncc/model_zoo/ncc/openvino_2021.4/person-detection-retail-0013/config/input_yuv420_720P.json"};

    /* 3.create handle, synchronous mode */
    ret = ncc_pipe_create(&handleFD, NCC_ASYNC);
    if(ret>0)
    {
        printf("ncc_pipe_create error %d !\n", ret);
        return -1;
    }

    ret = ncc_pipe_create(&handlePD, NCC_ASYNC);
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

    /* create face detection thread */
    static pthread_t threadFD;
    ret = pthread_create(&threadFD, NULL, FDSrcUvcCameraThread, (void*)&handleFD);
    if (ret)
    {
        printf("Error - pthread_create(FDSrcUvcCameraThread) return code: %d\n", ret);
        return -1;
    }
    /* create person detection thread */
    static pthread_t threadPD;
    ret = pthread_create(&threadPD, NULL, PDSrcDevCameraThread, (void*)&handlePD);
    if (ret)
    {
        printf("Error - pthread_create(PDSrcDevCameraThread) return code: %d\n", ret);
        return -1;
    }
    while(1)
    {
        /* refresh display */
        sem_wait(&videoDone);
        showAllVideo();
        continue;
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
    pthread_mutex_lock(&procMutex);
    cv::Mat img_yuv;
    img_yuv.create(h*3/2, w, CV_8UC1);

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

        cv::Point origin;
        origin.x = object.x ;
        origin.y = object.y ;
        char   result[128];
        memset(result, 0, sizeof(result));
        sprintf(result, "score:%d%%", (int)(100*box.conf));
        cv::putText(img_out, result, origin, cv::FONT_HERSHEY_COMPLEX, 0.8,  cv::Scalar(255, 255, 128), 1, 8, 0);
    }

    /* scale */
    resize(img_out,showImage[id], Size(img_out.cols*scale,img_out.rows*scale),0,0,INTER_LINEAR);
    strcpy(showname[id], name);
    showStart[id] = 1;

    pthread_mutex_unlock(&procMutex);
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

void showAllVideo(void)
{
    for(int i=0;i<MAX_DEV_NUM*MAX_PIPELINE_NUM;i++)
    {
        if(showStart[i])
            cv::imshow(showname[i], showImage[i]);
    }
    cv::waitKey(10);
}
