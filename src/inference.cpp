#include <iostream>
#include <MNN/Interpreter.hpp>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define FILE_NUM_MAX 5000
#define FILE_SIZE_MAX 5000
#include <opencv2/opencv.hpp>
#define path_size 521
char file_table[FILE_NUM_MAX][path_size];

cv::Mat pad(cv::Mat image, std::vector<int> padding, int fill=0, std::string padding_mode="constant") {
    int pad_left = padding[0];
    int pad_top = padding[1];
    int pad_right = padding[2];
    int pad_bottom = padding[3];

    cv::Mat padded_image;
    // printf("padding: %d %d %d %d\n", pad_left, pad_top, pad_right, pad_bottom);
    cv::copyMakeBorder(image, padded_image, pad_top, pad_bottom, pad_left, pad_right, cv::BORDER_CONSTANT, cv::Scalar(fill));

    return padded_image;
}

cv::Mat rescale(cv::Mat image, int output_size, int interpolation=cv::INTER_LINEAR) {
    int h = image.rows;
    int w = image.cols;

    double new_h, new_w;
    if (output_size > 0) {
        if (h < w) {
            new_h = output_size * h / w;
            new_w = output_size;
        } else {
            new_h = output_size;
            new_w = output_size * w / h;
        }
    } else {
        new_h = output_size;
        new_w = output_size;
    }

    cv::Mat resized_image;
    cv::resize(image, resized_image, cv::Size(static_cast<int>(new_w), static_cast<int>(new_h)), 0, 0, interpolation);

    return resized_image;
}

cv::Mat rescale_pad(cv::Mat image, int output_size, int interpolation=cv::INTER_LINEAR, int fill=0, std::string padding_mode="constant") {
    cv::Mat resized_image = rescale(image, output_size, interpolation);
    // return resized_image;
    int h = resized_image.rows;
    int w = resized_image.cols;

    int pad_left = (output_size - w) / 2;
    int pad_right = output_size - w - pad_left;
    int pad_top = (output_size - h) / 2;
    int pad_bottom = output_size - h - pad_top;

    std::vector<int> padding = {pad_left, pad_top, pad_right, pad_bottom};

    cv::Mat padded_image = pad(resized_image, padding, fill, padding_mode);

    return padded_image;
}


static int file_index = 0;
int set_table(const char *path)
{
	//定义一个 DIR 类的指针
	DIR *pDir;
	//定义一个结构体 dirent 的指针，dirent 结构体见上
	struct dirent *ent;
	int i = 0;
	//定义一个字符数组，用来存放读取的路径
	char childpath[512];

	pDir = opendir(path); //  opendir 方法打开 path 目录，并将地址付给 pDir 指针
	memset(childpath, 0, sizeof(childpath)); //将字符数组 childpath 的数组元素全部置零
						 //读取 pDir 打开的目录，并赋值给 ent, 同时判断是否目录为空，不为空则执行循环体
	while ((ent = readdir(pDir)) != NULL) {
		//读取 打开目录的文件类型 并与 DT_DIR 进行位与运算操作，即如果读取的 d_type 类型为 DT_DIR (=4 表示读取的为目录)
		if (ent->d_type & DT_DIR) {
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
				//如果读取的 d_name 为 . 或者.. 表示读取的是当前目录符和上一目录符, 用 contiue 跳过，不进行下面的输出
				continue;
			}
			//如果非. ..则将 路径 和 文件名 d_name 付给 childpath, 并在下一行 prinf 输出
			snprintf(childpath, 512, "%s/%s", path, ent->d_name);
			//printf("path:%s\n", childpath);
			//递归读取下层的字目录内容， 因为是递归，所以从外往里逐次输出所有目录（路径+目录名），
			//然后才在 else 中由内往外逐次输出所有文件名
			set_table(childpath);
		}
		//如果读取的 d_type 类型不是 DT_DIR, 即读取的不是目录，而是文件，则直接输出 d_name, 即输出文件名
		else {
			//printf("%s\n", ent->d_name);
			snprintf(file_table[file_index], path_size, "%s/%s", path, ent->d_name);
			//printf("d_name: %s\n", file_table[file_index]);
			file_index++;
		}
	}
	return file_index;
}

#define DEBUG 1

#if DEBUG
#define hp_printf(format, ...) \
       {printf("[%s : %s : %d] ", \
       __FILE__, __func__, __LINE__); \
       printf(format, ##__VA_ARGS__);}
#else
#define hp_printf(format, ...)
#endif

using namespace MNN;

long get_ms()
{
#if 0
	struct timeval timestamp = {};
	if (0 == gettimeofday(&timestamp, NULL))
	    return timestamp.tv_sec * 1000000 + timestamp.tv_usec;
	else
	    return 0;
#endif
	struct timeb timestamp = {};
	
	if (0 == ftime(&timestamp))
	    return timestamp.time * 1000 + timestamp.millitm;
	else
	    return 0;
}
char file_buf[2000000] = {0};
char tmpbuf[256];
FILE *g_fp;
int main(int argc, char *argv[])
{
	int flag = 0;
    long end, start, inf_sum = 0;
    long g_start = get_ms();
	if (argc != 4) {
		hp_printf("usage : ./main model dir_picture save\n");
		return 0;
	}
    start = g_start;
    	if (strcmp("save", argv[3]) == 0) {
		flag = 1;
		g_fp = fopen("./result.txt", "w");
		if (g_fp == NULL) {
			perror("fopen error\n");
			assert(0);	
		}
	}
    // const char *pchPath = "/home/pi/MNN/build/best_pref_607.mnn";
    //const char *pchPath = "/home/pi/MNN/build/s607.mnn";
    char * pchPath = argv[1];
    /*interpreter 解释器，是模型数据的持有者，我称之为net
     * session 会话，是推理数据的持有者，session通过interpreter创建，多个session可以公用一个interpreter
     * session 和TF的session类似
     * 我们需要通过net创建session1、session2等会话，然后再给session里送入数据，
     * 最后通过net->runSession(session)执行推理
    */
    std::shared_ptr<Interpreter> net(Interpreter::createFromFile(pchPath));

    // 创建session
    ScheduleConfig config;
    config.type  = MNN_FORWARD_AUTO;
    Session *pSession = net->createSession(config);
    // 获取输入Tensor
    // getSessionInput 用于获取单个输入tensor
    Tensor *ptensorInput = net->getSessionInput(pSession, NULL);
    std::vector<int> vctInputDims = ptensorInput->shape();
   
    // printf("输入Tensor的维度为： ");
    for (size_t i = 0; i < vctInputDims.size(); ++i)
    {
        printf("%d ", vctInputDims[i]);
    }
    printf("\n");
	end = get_ms();
	hp_printf("create session time : %ld ms\n", (end - start)); 
	start = end;
//    shape[0]   = 1;
    // 重建Tensor的形状，我们这里输入的固定的224*224， 所以可以不需要resize
//    net->resizeTensor(tensorInput, vctInputShape);
//    net->resizeSession(pSession);
    // opencv 读取数据，resize操作，减均值， 除方差，并且转成nchw
    //char * path = "/media/mclab207/Datas/yc/images_101";
    char * path = argv[2];
	int fn = set_table(path);
	for (int iter = 0; iter < fn; iter++) {
    cv::Mat matBgrImg = cv::imread(file_table[iter]);
    cv::Mat matRgbImg;
    cv::cvtColor(matBgrImg, matRgbImg, cv::COLOR_BGR2RGB);
    cv::Mat matRgbRescaleImg;
    matRgbRescaleImg = rescale_pad(matRgbImg, 96);
    matRgbRescaleImg.convertTo(matRgbRescaleImg, CV_32FC3);
    cv::Mat matNormImage;
    cv::Mat matRzRgbImage, matFloatImage;
    int MODEL_INPUT_HEIGHT = 96;
    int MODEL_INPUT_WIDTH = 96;
    cv::Mat matStd(MODEL_INPUT_HEIGHT, MODEL_INPUT_WIDTH, CV_32FC3, cv::Scalar(58.395f, 57.12f, 57.375f));
    cv::resize(matBgrImg, matRzRgbImage, cv::Size(MODEL_INPUT_WIDTH, MODEL_INPUT_HEIGHT));
    matRzRgbImage.convertTo(matFloatImage, CV_32FC3);
    cv::Mat matMean(MODEL_INPUT_HEIGHT, MODEL_INPUT_WIDTH, CV_32FC3, \
                        cv::Scalar(123.675f, 116.28f, 103.53f)); // 均值

    matNormImage = (matRgbRescaleImg - matMean) / matStd;
    cv::Mat OnesImage(96, 96, CV_32FC3, cv::Scalar(1.0f, 2.0f, 3.0f));
    std::vector<std::vector<cv::Mat>> nChannels;
    std::vector<cv::Mat> rgbChannels(3);
    cv::split(matNormImage, rgbChannels);
    nChannels.push_back(rgbChannels); //  NHWC  转NCHW
    //void *pvData = malloc(1 * 3 * MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH *sizeof(float));
    char pvData[1 * 3 * MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH *sizeof(float)];
    int nPlaneSize = MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH;
    for (int c = 0; c < 3; ++c)
    {
        cv::Mat matPlane = nChannels[0][c];
        memcpy((float *)(pvData) + c * nPlaneSize,\
             matPlane.data, nPlaneSize * sizeof(float));
    }

    // 将数据拷贝到Tensor中
    // 这里采用最简洁的输入方式，直接利用host填充数据，但这种方式仅限于CPU后端，其他后端
    // 还是需要通过deviceid输入。此外，用这种方式填充数据需要我们自行处理NCHW或者NHWC的数据格式
    // 本文这里，已经将NHWC转成了NCHW了，即 pvData
//    std::shared_ptr<MNN::CV::ImageProcess> pretreat_data_ = nullptr;
    auto nchwTensor = new Tensor(ptensorInput, Tensor::CAFFE);
    ::memcpy(nchwTensor->host<float>(), pvData, nPlaneSize * 3 * sizeof(float));
    ptensorInput->copyFromHostTensor(nchwTensor);
    delete nchwTensor;

	end = get_ms();
	//hp_printf("process picture time : %ld ms\n", (end - start)); 
	start = end;
    // 获取输出Tensor
	long inf_start = get_ms();
    Tensor *pTensorOutput = net->getSessionOutput(pSession, NULL);

    // 执行推理
    net->runSession(pSession);
    {
        // 获取输出维度类型
        auto dimType = pTensorOutput->getDimensionType();
        if (pTensorOutput->getType().code != halide_type_float) {
            dimType = Tensor::CAFFE;
        }
        // 创建输出tensor
        std::shared_ptr<Tensor> outputUser(new Tensor(pTensorOutput, dimType));
        // MNN_PRINT("output size:%d\n", outputUser->elementSize());
        // 拷贝出去
        pTensorOutput->copyToHostTensor(outputUser.get());
        auto type = outputUser->getType();

        auto size = outputUser->elementSize();
        std::vector<std::pair<int, float>> tempValues(size);
        if (type.code == halide_type_float) {
            auto values = outputUser->host<float>();
            for (int i = 0; i < size; ++i) {
                tempValues[i] = std::make_pair(i, values[i]);
            }
        }
        // Find Max
        // 排序, 打印
        std::sort(tempValues.begin(), tempValues.end(),
                  [](std::pair<int, float> a, std::pair<int, float> b) { return a.second > b.second; });
#if 0
        int length = size > 10 ? 10 : size;
        for (int i = 0; i < length; ++i) {
            MNN_PRINT("%d, %f\n", tempValues[i].first, tempValues[i].second);
        }
#endif
	// sprintf(tmpbuf, "%s %d %f\n", file_table[iter], tempValues[0].first, tempValues[0].second);
	if (flag == 0) {
    		printf("%s %d\n", file_table[iter], tempValues[0].first);
	} else {
    		snprintf(tmpbuf, 256, "%s %d\n", file_table[iter], tempValues[0].first);
		strcat(file_buf, tmpbuf );
	}
    }
#if 0
    // 释放我们创建的数据内存,这个不是tensor里的
    if (NULL != pvData)
    {
        free (pvData);
        pvData = NULL;
    }
#endif
    // std::cout << "hello world " << std::endl;

	end = get_ms();
	//hp_printf("inference & post process time : %ld ms\n", (end - start)); 
	inf_sum += (end - inf_start);
}

	if (flag ==1 ) {
		fputs(file_buf, g_fp);
		fclose(g_fp);
	}
    long g_end = get_ms();
    hp_printf("all process time : %ld ms, start time: %ld ms, end time: %ld ms \n",
		 (g_end - g_start), g_start, g_end);
	hp_printf("inf_time arvg: %f ms\n", (inf_sum/100.));
    return 0;
}

