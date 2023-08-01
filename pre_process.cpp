#include <iostream>
#include <MNN/Interpreter.hpp>
#include <stdio.h>
#include <cstdio>
using namespace MNN;
#include <pre_process.hpp>
#include <opencv2/opencv.hpp>
//const char *pchPath = "/home/pi/MNN/build/best_pref.mnn";
char *pchPath;
#define g_size 1024 * 1024 * 10
char g_buf[g_size];
char tmpbuf[256];

#include <unistd.h>
#include <sys/syscall.h> 

pid_t gettid(void)
{
	return syscall(SYS_gettid);
}

#if 1
Tensor *ptensorInput[DIVISOR + 1];
Session *pSession[DIVISOR + 1];
std::shared_ptr<Interpreter> net[DIVISOR + 1];//(Interpreter::createFromFile(pchPath));
int session_init(char * path, int which)
{
	hp_printf("%d, model path %s\n", which, path);
	/*interpreter 解释器，是模型数据的持有者，我称之为net[which]
	 * session 会话，是推理数据的持有者，session通过interpreter创建，多个session可以公用一个interpreter
	 * session 和TF的session类似
	 * 我们需要通过net[which]创建session1、session2等会话，然后再给session里送入数据，
	 * 最后通过net[which]->runSession(session)执行推理
	 */
	pchPath = path;
	//hp_printf("%s\n", pchPath);
	// 创建session
	std::shared_ptr<Interpreter> net_copy(Interpreter::createFromFile(pchPath));
	if (NULL == net_copy) {
		hp_printf("create net error\n");
	}
	net[which] = net_copy;
	ScheduleConfig config;
	config.type  = MNN_FORWARD_AUTO;
	pSession[which] = net[which]->createSession(config);
	// 获取输入Tensor
	// getSessionInput 用于获取单个输入tensor
	ptensorInput[which] = net[which]->getSessionInput(pSession[which], NULL);
	if (net[which] == NULL || ptensorInput[which] == NULL) {
		hp_printf(">>>>>>>>>>> %d error\n", which);
	} else {
		//hp_printf(">>>>>>>>>>> %d %p %p success\n", which, (void *)net[which], ptensorInput[which]);
	}

	std::vector<int> vctInputDims = ptensorInput[which]->shape();

	return 0;
}
#endif
int picture_process(const char *path, int which)
{
	if (net[which] == NULL || ptensorInput[which] == NULL) {
		hp_printf(">>>>>>>>>>>%d error\n", which);
	} else {
		hp_printf("success %d\n", which);
	}

    // opencv 读取数据，resize操作，减均值， 除方差，并且转成nchw
    cv::Mat matBgrImg = cv::imread(path);
    hp_printf("[pid width height pthread] %p %d %d %d\n", pthread_self(), matBgrImg.rows, matBgrImg.cols, which);
    cv::Mat matNormImage;
    cv::Mat matRzRgbImage, matFloatImage;
    int MODEL_INPUT_HEIGHT = 96;
    int MODEL_INPUT_WIDTH = 96;
    cv::Mat matStd(MODEL_INPUT_HEIGHT, MODEL_INPUT_WIDTH, CV_32FC3, cv::Scalar(57.375f, 57.12f, 58.395f));
    cv::resize(matBgrImg, matRzRgbImage, cv::Size(MODEL_INPUT_WIDTH, MODEL_INPUT_HEIGHT));
    matRzRgbImage.convertTo(matFloatImage, CV_32FC3);
    cv::Mat matMean(MODEL_INPUT_HEIGHT, MODEL_INPUT_WIDTH, CV_32FC3, \
                        cv::Scalar(103.53f, 116.28f, 123.675f)); // 均值

    matNormImage = (matFloatImage - matMean) / matStd;
    cv::Mat OnesImage(96, 96, CV_32FC3, cv::Scalar(1.0f, 2.0f, 3.0f));
    std::vector<std::vector<cv::Mat>> nChannels;
    std::vector<cv::Mat> rgbChannels(3);
    cv::split(matNormImage, rgbChannels);
    nChannels.push_back(rgbChannels); //  NHWC  转NCHW
    int size = 1 * 3 * MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH *sizeof(float);
    char pvData[size];
    //char *pvData = (char *) malloc(size);
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
    auto nchwTensor = new Tensor(ptensorInput[which], Tensor::CAFFE);
    ::memcpy(nchwTensor->host<float>(), pvData, nPlaneSize * 3 * sizeof(float));
    ptensorInput[which]->copyFromHostTensor(nchwTensor);
    delete nchwTensor;

    // 获取输出Tensor
    Tensor *pTensorOutput = net[which]->getSessionOutput(pSession[which], NULL);

    // 执行推理
    net[which]->runSession(pSession[which]);
    {
        // 获取输出维度类型
        auto dimType = pTensorOutput->getDimensionType();
        if (pTensorOutput->getType().code != halide_type_float) {
            dimType = Tensor::CAFFE;
        }
        // 创建输出tensor
        std::shared_ptr<Tensor> outputUser(new Tensor(pTensorOutput, dimType));
        //MNN_PRINT("output size:%d\n", outputUser->elementSize());
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
	//hp_printf("%s %d %f", path, tempValues[0].first, tempValues[0].second);
	if (g_flag == 0) {
		printf("%s %d\n", path, tempValues[0].first);
	} else {

		snprintf(tmpbuf, 256, "%s %d\n", path, tempValues[0].first);
		strcat(g_buf, tmpbuf);
	}

    }
    return 0;
}
