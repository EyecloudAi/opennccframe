#include <includes.h>
#include <cameraCtrl.h>
#include <native_vpu_api.h>

#include <linux/videodev2.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>

using namespace cv;

typedef struct
{
    void *start;
    size_t length;
} VideoSpec_t;

int  v4l2_device_open(char *node);
int  v4l2_device_start(int fd);
int  v4l2_device_fill(int fd, struct v4l2_buffer *src);
int  v4l2_device_queue(int fd, struct v4l2_buffer *src);
void v4l2_device_query(int fd, int *imageWidth, int *imageHeight);
void v4l2_device_port_get(int fd, NccUsbPortSpec_t *port);
VideoSpec_t* v4l2_device_allocate(int fd, struct v4l2_buffer *src);

void opencv_show_img_func(void *data, int w, int h, float scale, char *name);

int main(int argc, char* argv[])
{
    if(argc <2)
    {
        printf("Please input sudo ./example_video_control_app [<node(/dev/video*)>]");
        return 0;
    }

    int ret = 0;
    /* 1.detect device */
    int devNum = ncc_dev_number_get();
    printf("ncc_dev_number_get find devs %d\n", devNum);
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

    /* 3. start device */
    ret = ncc_dev_start(0);
    if(ret>0)
    {
        printf("ncc_dev_start error %d !\n", ret);
        return -1;
    }

    int imageWidth, imageHeight;

    /* 4. open UVC camera*/
    int fd;
    fd = v4l2_device_open(argv[1]);

    /* 5. get usb port */
    NccUsbPortSpec_t port;
    memset(&port, 0, sizeof(NccUsbPortSpec_t));
    v4l2_device_port_get(fd, &port);

    /* 6. get sn from port */
    char str[32];
    ncc_dev_serial_number_get(&port, str, sizeof(str));

    /* 7. get device controller id */
    int dev_id = device_ctrl_id_get(str);

    /* 8. get device information */
    char version[100];
    memset(version, 0, sizeof(version));
    device_ctrl_get_fw_version(dev_id, version, sizeof(version));
    printf("Firmware version: %s\n\n",version);

    /* 9. set manual exposure (this step is not necessary)*/
    camera_ctrl_me_set_exp_gain(dev_id, 5000, 400);

    /* 10. query camera video format，VIDIOC_ENUM_FMT */
    v4l2_device_query(fd, &imageWidth, &imageHeight);

    struct v4l2_buffer src;
    VideoSpec_t *frame = v4l2_device_allocate(fd, &src);
    if(frame == 0)
        exit(0);

    /* 11. start capturing images */
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

        /*2. get frame index  */
        unsigned int *pSeqNo = (unsigned int *)frame[src.index].start;

        /*3. display  */
        char str[128];
        float scale = 1.0*960/imageWidth;
        sprintf(str, "video_capture");
        opencv_show_img_func((char*)frame[src.index].start,imageWidth, imageHeight, scale, str);

        /* 4.The frame buffer is re-queued into the input queue. */
        ret = v4l2_device_queue(fd, &src);
        if(ret == -1)
        {
            perror("VIDIOC_QBUF failed!\n");
            continue;
        }
    }
}

void  opencv_show_img_func(void *data, int w, int h, float scale, char *name)
{
    cv::Mat yuvImg;
    yuvImg.create(h * 3 / 2, w, CV_8UC1);
    cv::Mat yuvImg_resized;
    cv::Mat outgoing_img;

    /* YUV420P-->RGB */
    yuvImg.data = (unsigned char*)data;
    cv::cvtColor(yuvImg, outgoing_img, CV_YUV2BGR_I420);

    Mat showImage;
    /* Zoom display */
    resize(outgoing_img,showImage,Size(outgoing_img.cols*scale,outgoing_img.rows*scale),0,0,INTER_LINEAR);
    cv::imshow(name, showImage);
    cv::waitKey(1);
}

int v4l2_device_open(char *node)
{
    int fd;
    fd = open(node,O_RDWR);

    return fd;
}

void v4l2_device_port_get(int fd, NccUsbPortSpec_t *port)
{
    struct v4l2_capability cap;
    ioctl(fd,VIDIOC_QUERYCAP,&cap);

    char result[16];
    const char *split = "-";
    char *str;
    str = strtok((char*)cap.bus_info, split);
    while(str != NULL)
    {
        memset(result,0,sizeof(result));
        strcpy(result, str);
        str = strtok(NULL, split);
    }

    const char *split2 = ".";
    char *str2;
    str2 = strtok(result, split2);
    while(str2 != NULL)
    {
        port->path[port->length] = atoi(str2);
        port->length++;
        str2 = strtok(NULL, split2);
    }
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
    printf("Support format:\n");
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
