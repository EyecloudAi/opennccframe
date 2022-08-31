#include <includes.h>
#include <cameraCtrl.h>
#include <native_vpu_api.h>

#include <linux/videodev2.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <malloc.h>
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
} OvDetectSpec_t;

cv::Mat showImage[MAX_DEV_NUM * MAX_PIPELINE_NUM];
char showname[MAX_DEV_NUM * MAX_PIPELINE_NUM][100];
int showStart[MAX_DEV_NUM * MAX_PIPELINE_NUM] = {0};

cv::Mat cv_image_import(char *filepath, int *imageWidth, int *imageHeight);
void cv_tensor_convert(cv::Mat src, char *output, int width, int height);

void obj_show_img_func(char *data, int w, int h, float scale, char *name, float *pMeta, cv::Rect object, int id);
void showAllVideo(void);

int coordinate_is_valid(float x1, float y1, float x2, float y2)
{
    if ((x1 < 0) || (x1 > 1))
        return 0;
    if ((y1 < 0) || (y1 > 1))
        return 0;
    if ((x2 < 0) || (x2 > 1))
        return 0;
    if ((y2 < 0) || (y2 > 1))
        return 0;
    if ((x1 >= x2) || (y1 >= y2))
        return 0;

    return 1;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    int testseq = 0;

    /* 1.detect device */
    int devNum = ncc_dev_number_get();
    printf("nccLinkInit find devs %d\n", devNum);
    if (devNum <= 0)
    {
        printf("ncc_dev_number_get error, the device was not found\n");
        return -1;
    }
    //   int devNum=1;
    /* 2.initialize device, load firmware */
    // ret = ncc_dev_init(NULL, devNum);
    ret = ncc_dev_init("/usr/lib/openncc/OpenNcc.mvcmd", devNum);
    if (ret < 0)
    {
        printf("ncc_dev_init error\n");
        return -1;
    }
    else
    {
        printf("ncc_dev_init success num %d\n", ret);
    }

    NccPipeHandle_t handleFD = {NULL,
                                "",
                                "face-detection-retail-0004",
                                "/usr/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/face-detection-retail-0004.blob",
                                "/usr/lib/openncc/model_zoo/ncc/openvino_2021.4/face-detection-retail-0004/config/input_BGR.json"};

    NccPipeHandle_t handleAG = {NULL,
                                "",
                                "age-gender-recognition-retail-0013",
                                "/usr/lib/openncc/model_zoo/ncc/openvino_2021.4/age-gender-recognition-retail-0013/age-gender-recognition-retail-0013.blob",
                                "/usr/lib/openncc/model_zoo/ncc/openvino_2021.4/age-gender-recognition-retail-0013/config/input_BGR.json"};

    /* 3.create handle, synchronous mode */
    ret = ncc_pipe_create(&handleFD, NCC_SYNC);
    if (ret > 0)
    {
        printf("ncc_pipe_create error %d !\n", ret);
        return -1;
    }

    ret = ncc_pipe_create(&handleAG, NCC_SYNC);
    if (ret > 0)
    {
        printf("ncc_pipe_create error %d !\n", ret);
        return -1;
    }

    /* 4.start device */
    for (int i = 0; i < devNum; i++)
    {
        ret = ncc_dev_start(i);
        if (ret > 0)
        {
            printf("ncc_dev_start error %d !\n", ret);
            return -1;
        }
    }

    /* get input size of model */
    NccTensorSpec_t input1;
    ncc_input_tensor_descriptor_get(&handleFD, &input1);
    int inputDimWidth1 = input1.tensor[3];
    int inputDimHeight1 = input1.tensor[2];
    int inputsize1 = inputDimWidth1 * inputDimHeight1 * 3;

    NccTensorSpec_t input2;
    ncc_input_tensor_descriptor_get(&handleAG, &input2);
    int inputDimWidth2 = input2.tensor[3];
    int inputDimHeight2 = input2.tensor[2];
    int inputsize2 = inputDimWidth2 * inputDimHeight2 * 3;

    int imageWidth;
    int imageHeight;

    VideoCapture cap(0);
    while (1)
    {
        /* import image */
        cv::Mat cv_img;
        cap.read(cv_img);

        if (cv_img.empty())
        {
            return -1;
        }
        imageWidth = cv_img.cols;
        imageHeight = cv_img.rows;
        int maxOutSize = 1024 * 1024;

        NccPipeInput_t *pInData1 = ncc_malloc(inputsize1);
        NccPipeOutput_t pOutData1;
        pOutData1.output = malloc(maxOutSize);

        // printf("size %d\n", malloc_usable_size(pOutData1.output));

        pOutData1.alloc_size = maxOutSize;
        cv_tensor_convert(cv_img, (char *)pInData1->input, inputDimWidth1, inputDimHeight1);

        ret = sync_process(&handleFD, pInData1, &pOutData1, 10000);

        float *pMeta = (float *)pOutData1.output;

        cv::Mat img_out = cv_img;
        int i, dis_w, dis_h, oft_x, oft_y;
        /* get fov */
        oft_x = 0;
        oft_y = 0;
        dis_w = imageWidth;
        dis_h = imageHeight;

        for (int i = 0; i < 200; i++)
        {

            OvDetectSpec_t box;
            memcpy(&box, &pMeta[i * 7], sizeof(OvDetectSpec_t));

            /* eliminate invalid data and low score */
            if ((coordinate_is_valid(box.x_min, box.y_min, box.x_max, box.y_max) == 0) || (box.conf < 0.5) || box.image_id < 0)
            {
                continue;
            }

            /* draw box */
            cv::Rect object;
            object.x = box.x_min * dis_w + oft_x;
            object.y = box.y_min * dis_h + oft_y;
            object.width = (box.x_max - box.x_min) * dis_w;
            object.height = (box.y_max - box.y_min) * dis_h;

            //      printf("box %d %d %d %d,dis_w(%d %d)\n", object.x,object.y, object.x +object.width,object.y+object.height, dis_w,dis_h);
            cv::rectangle(img_out, object, cv::Scalar(255, 128, 128), 2, 8, 0);

            cv::Mat face = cv_img(Range((int)(imageHeight * box.y_min), (int)(imageHeight * box.y_max)), Range((int)(imageWidth * box.x_min), (int)(imageWidth * box.x_max)));

            /* allocates memory for storing images sent to the device */
            NccPipeInput_t *pInData2 = ncc_malloc(inputsize2);
            if (pInData2 == 0)
            {
                printf("ncc_malloc error\n");
                return 0;
            }

            pInData2->seqNo = testseq++;
            // while(1)

            /* convert BGR_i --> BGR_P */
            cv_tensor_convert(face, (char *)pInData2->input, inputDimWidth2, inputDimHeight2);

            pInData2->size = inputsize2;

            NccPipeOutput_t pOutData2;
            int maxOutSize = 1024 * 1024;

            /* allocates memory for storing meta data */

            pOutData2.output = malloc(maxOutSize);
            if (pOutData2.output == 0)
            {
                printf("'malloc error\n");
                return 0;
            }
            pOutData2.alloc_size = maxOutSize;

            ret = sync_process(&handleAG, pInData2, &pOutData2, 10000);

            // printf("%s, inSeq:%d outSeq:%d outSize:%d ,T:%d MS, pid:%d\n", handleAG.pipe_name, pInData2->seqNo,
            //        pOutData2.seqNo, pOutData2.actual_size, (int)pOutData2.elapsed_time, ncc_pipe_id_get(&handleAG));

            /* display */
            char str[128];
            float scale = 1.0 * 960 / imageWidth;
            float min_score = 0.5;
            sprintf(str, "%s", handleAG.pipe_name);
            obj_show_img_func((char *)img_out.data, imageWidth, imageHeight, scale, str, (float *)pOutData2.output, object, ncc_pipe_id_get(&handleAG));

            /* free memory */
            ncc_free(pInData2);
            free(pOutData2.output);
            pOutData2.output = 0;
            pInData2 = 0;
        }
        ncc_free(pInData1);
        free(pOutData1.output);
        pOutData1.output = 0;
        pInData1 = 0;
        cv::imshow("out", img_out);
        cv::waitKey(1);

        /* Invoke the synchronous inference interface，overtime is 10s */
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

    *imageWidth = cv_img.cols;
    *imageHeight = cv_img.rows;

    return cv_img;
}

void cv_tensor_convert(cv::Mat src, char *output, int width, int height)
{
    cv::Mat dest;

    /* scale to input size according to model */
    resize(src, dest, Size(width, height));

    /* BGR_i-->BGR_P */
    char *p_b = output;
    char *p_g = output + width * height;
    char *p_r = output + width * height * 2;
    char *p_in = (char *)(dest.data);

    for (int i = 0; i < width * height; i++)
    {
        p_b[i] = p_in[3 * i + 0];
        p_g[i] = p_in[3 * i + 1];
        p_r[i] = p_in[3 * i + 2];
    }
}

void obj_show_img_func(char *img, int w, int h, float scale, char *name, float *pMeta, cv::Rect object, int id)
{
    cv::Mat img_out;
    img_out.create(h, w, CV_8UC3);
    int i, dis_w, dis_h, oft_x, oft_y;

    img_out.data = (unsigned char *)img;

    /* get fov */
    oft_x = 0;
    oft_y = 0;
    dis_w = w;
    dis_h = h;

    float pMeta1 = pMeta[0];
    float pMeta2 = pMeta[32];
    float pMeta3 = pMeta[33];

    cv::Point origin;
    origin.x = object.x;
    origin.y = object.y;
    char result[128];
    memset(result, 0, sizeof(result));
    if (pMeta3 >= pMeta2)
    {
        sprintf(result, "male, %d  ", (int)(pMeta1 * 100));
    }
    else
    {
        sprintf(result, "female, %d  ", (int)(pMeta1 * 100));
    }

    cv::putText(img_out, result, origin, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 255, 128), 1, 8, 0);
    // }
}
