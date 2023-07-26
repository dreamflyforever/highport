#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;

int main()
{
	cv::Mat matBgrImg = cv::imread("/media/mclab207/Datas/yc/ESFair2023/TrainingSet    /G6/NV/0032069.jpg");
	cv::Mat matNormImage;
	cv::Mat matRzRgbImage, matFloatImage;
	int MODEL_INPUT_HEIGHT = 128;
	int MODEL_INPUT_WIDTH = 128;
	cv::Mat matStd(MODEL_INPUT_HEIGHT, MODEL_INPUT_WIDTH, CV_32FC3, cv::Scalar(57.375f, 57.12f, 58.395f));
	cv::resize(matBgrImg, matRzRgbImage, cv::Size(MODEL_INPUT_WIDTH, MODEL_INPUT_HEIGHT));
	matRzRgbImage.convertTo(matFloatImage, CV_32FC3);
	cv::Mat matMean(MODEL_INPUT_HEIGHT, MODEL_INPUT_WIDTH, CV_32FC3, \
	                    cv::Scalar(103.53f, 116.28f, 123.675f)); // 均值
	
	matNormImage = (matFloatImage - matMean) / matStd;
	
	std::vector<std::vector<cv::Mat>> nChannels;
	std::vector<cv::Mat> rgbChannels(3);
	cv::split(matNormImage, rgbChannels);
	nChannels.push_back(rgbChannels); //  NHWC  转NCHW
	void *pvData = malloc(1 * 3 * MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH *sizeof(float));
	int nPlaneSize = MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH;
	for (int c = 0; c < 3; ++c)
	{
	    cv::Mat matPlane = nChannels[0][c];
	    memcpy((float *)(pvData) + c * nPlaneSize,\
	         matPlane.data, nPlaneSize * sizeof(float));
	}
	
	// 将数据拷贝到Tensor中
	// 这里采用最简洁的输入方式，直接利用host填充数据，但这种方式仅限于CPU后端，其>    他后端
	// 还是需要通过deviceid输入。此外，用这种方式填充数据需要我们自行处理NCHW或者NH    WC的数据格式
	// 本文这里，已经将NHWC转成了NCHW了，即 pvData
	//std::shared_ptr<MNN::CV::ImageProcess> pretreat_data_ = nullptr;
	auto nchwTensor = new Tensor(ptensorInput, Tensor::CAFFE);
	::memcpy(nchwTensor->host<float>(), pvData, nPlaneSize * 3 * sizeof(float));
	ptensorInput->copyFromHostTensor(nchwTensor);
	delete nchwTensor;
	
	// 获取输出Tensor
	Tensor *pTensorOutput = net->getSessionOutput(pSession, NULL);
	return 0;
}
