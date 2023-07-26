#include <iostream>
#include <MNN/Interpreter.hpp>

using namespace MNN;

#include <opencv2/opencv.hpp>
const char *pchPath = "/home/pi/MNN/build/best_pref.mnn";
#if 1
Tensor *ptensorInput;
Session *pSession;
std::shared_ptr<Interpreter> net(Interpreter::createFromFile(pchPath));
int times = 0;
int session_init()
{
	if (times == 0) {
		times = 1;
		/*interpreter 解释器，是模型数据的持有者，我称之为net
		 * session 会话，是推理数据的持有者，session通过interpreter创建，多个session可以公用一个interpreter
		 * session 和TF的session类似
		 * 我们需要通过net创建session1、session2等会话，然后再给session里送入数据，
		 * 最后通过net->runSession(session)执行推理
		 */
	
		// 创建session
		ScheduleConfig config;
		config.type  = MNN_FORWARD_AUTO;
		pSession = net->createSession(config);
		// 获取输入Tensor
		// getSessionInput 用于获取单个输入tensor
		ptensorInput = net->getSessionInput(pSession, NULL);
		std::vector<int> vctInputDims = ptensorInput->shape();
		printf("输入Tensor的维度为%d： ", times);
		for (size_t i = 0; i < vctInputDims.size(); ++i)
		{
			printf("%d ", vctInputDims[i]);
		}
		printf("\n");
	}
	return 0;
}
#endif
int picture_process(const char *path)
{
#if 1
    //session_init();
#else    
    std::shared_ptr<Interpreter> net(Interpreter::createFromFile(pchPath));

    // 创建session
    ScheduleConfig config;
    config.type  = MNN_FORWARD_AUTO;
    Session *pSession = net->createSession(config);
    // 获取输入Tensor
    // getSessionInput 用于获取单个输入tensor
    Tensor *ptensorInput = net->getSessionInput(pSession, NULL);
    std::vector<int> vctInputDims = ptensorInput->shape();

//    shape[0]   = 1;
    // 重建Tensor的形状，我们这里输入的固定的224*224， 所以可以不需要resize
//    net->resizeTensor(tensorInput, vctInputShape);
//    net->resizeSession(pSession);
#endif
    // opencv 读取数据，resize操作，减均值， 除方差，并且转成nchw
    cv::Mat matBgrImg = cv::imread(path);
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
    cv::split(OnesImage, rgbChannels);
    nChannels.push_back(rgbChannels); //  NHWC  转NCHW
    int size = 1 * 3 * MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH *sizeof(float);
    char pvData[size];
    //char pvData[100000] = {0};
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

    // 获取输出Tensor
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
        MNN_PRINT("output size:%d\n", outputUser->elementSize());
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

        int length = size > 10 ? 10 : size;
        for (int i = 0; i < length; ++i) {
            MNN_PRINT("%d, %f\n", tempValues[i].first, tempValues[i].second);
        }

    }
#if 0
    // 释放我们创建的数据内存,这个不是tensor里的
    if (NULL != pvData)
    {
        free (pvData);
        pvData = NULL;
    }
    std::cout << "hello world " << std::endl;
#endif
    return 0;
}


// #include <iostream>
// #include <MNN/Interpreter.hpp>
// 
// using namespace MNN;
// 
// #include <opencv2/opencv.hpp>
// 
// int main()
// {
//     const char *pchPath = "/home/pi/MNN/build/mobileone_128pix.mnn";
//     std::shared_ptr<Interpreter> net(Interpreter::createFromFile(pchPath));
// 
//     ScheduleConfig config;
//     config.type  = MNN_FORWARD_AUTO;
//     Session *pSession = net->createSession(config);
// 
//     Tensor *ptensorInput = net->getSessionInput(pSession, NULL);
//     std::vector<int> vctInputDims = ptensorInput->shape();
//     printf("输入Tensor的维度为： ");
//     for (size_t i = 0; i < vctInputDims.size(); ++i)
//     {
//         printf("%d ", vctInputDims[i]);
//     }
//     printf("\n");
// 
//     cv::Mat matBgrImg = cv::imread("/media/mclab207/Datas/yc/ESFair2023/TrainingSet/G6/NV/0032069.jpg");
//     cv::Mat matRgbImg;
//     cv::cvtColor(matBgrImg, matRgbImg, cv::COLOR_BGR2RGB);
// 
//     // 进行MNN推理之前的处理...
// 
//     // 将RGB图像转换为NCHW格式
//     int MODEL_INPUT_HEIGHT = 128;
//     int MODEL_INPUT_WIDTH = 128;
//     cv::resize(matRgbImg, matRgbImg, cv::Size(MODEL_INPUT_WIDTH, MODEL_INPUT_HEIGHT));
//     cv::Mat matFloatImage;
//     matRgbImg.convertTo(matFloatImage, CV_32FC3);
// 
//     std::vector<cv::Mat> rgbChannels(3);
//     cv::split(matFloatImage, rgbChannels);
// 
//     void *pvData = malloc(1 * 3 * MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH * sizeof(float));
//     int nPlaneSize = MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH;
//     for (int c = 0; c < 3; ++c)
//     {
//         cv::Mat matPlane = rgbChannels[c];
//         memcpy((float *)(pvData) + c * nPlaneSize, matPlane.data, nPlaneSize * sizeof(float));
//     }
// 
//     // 将数据拷贝到Tensor中
//     auto nchwTensor = new Tensor(ptensorInput, Tensor::CAFFE);
//     ::memcpy(nchwTensor->host<float>(), pvData, nPlaneSize * 3 * sizeof(float));
//     ptensorInput->copyFromHostTensor(nchwTensor);
//     delete nchwTensor;
// 
//     // 获取输出Tensor
//     Tensor *pTensorOutput = net->getSessionOutput(pSession, NULL);
// 
//     net->runSession(pSession);
//     {
//         // 获取输出维度类型
//         auto dimType = pTensorOutput->getDimensionType();
//         if (pTensorOutput->getType().code != halide_type_float) {
//             dimType = Tensor::CAFFE;
//         }
//         // 创建输出tensor
//         std::shared_ptr<Tensor> outputUser(new Tensor(pTensorOutput, dimType));
//         MNN_PRINT("output size:%d\n", outputUser->elementSize());
//         // 拷贝出去
//         pTensorOutput->copyToHostTensor(outputUser.get());
//         auto type = outputUser->getType();
// 
//         auto size = outputUser->elementSize();
//         std::vector<std::pair<int, float>> tempValues(size);
//         if (type.code == halide_type_float) {
//             auto values = outputUser->host<float>();
//             for (int i = 0; i < size; ++i) {
//                 tempValues[i] = std::make_pair(i, values[i]);
//             }
//         }
//         // Find Max
//         // 排序, 打印
//         std::sort(tempValues.begin(), tempValues.end(),
//                   [](std::pair<int, float> a, std::pair<int, float> b) { return a.second > b.second; });
// 
//         int length = size > 10 ? 10 : size;
//         for (int i = 0; i < length; ++i) {
//             MNN_PRINT("%d, %f\n", tempValues[i].first, tempValues[i].second);
//         }
// 
//     }
// 
//     // 释放内存
//     if (NULL != pvData)
//     {
//         free(pvData);
//         pvData = NULL;
//     }
// 
//     std::cout << "hello world " << std::endl;
//     return 0;
// }
