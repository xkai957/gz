#include <torch/script.h> // One-stop header.
#include <iostream>
#include <memory>
#include <stdlib.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <atlconv.h>
#include <iostream>
#include <Util/logger.h>

#include "KConfig.h"

#include "CpytorchExcutor.h"


using namespace toolkit;
using namespace std;


namespace kmod
{

	CpytorchExcutor::CpytorchExcutor()
	{

	}
	CpytorchExcutor::~CpytorchExcutor()
	{

	}

	int CpytorchExcutor::load_netparam( std::string mycfgpath)
	{




		torch::jit::script::Module module = torch::jit::load(mycfgpath);


		//生成一个随机输入
		std::vector<torch::jit::IValue> inputs;
		inputs.emplace_back(torch::rand({ 64, 3, 224, 224 }));
		// 计算网络一次前向传播的需要时间
		auto t = (double)cv::getTickCount();
		torch::Tensor output = module.forward(std::move(inputs)).toTensor();
		t = (double)cv::getTickCount() - t;
		DebugL<<"execution time = "<< t / cv::getTickFrequency();
		inputs.pop_back();

		// 记载一张图片并且进行归一化
		cv::Mat image;
		image = cv::imread("imagename", 1);
		//转化为RGB三通道
		cv::cvtColor(image, image, cv::COLOR_BGR2RGB);  
		cv::Mat img_float;
		//首先归一化到[0,1]区间
		image.convertTo(img_float, CV_32F, 1.0 / 255);   
		//resize to 224，预训练的模型输入是batchsize x3 x 224 x 224
		cv::resize(img_float, img_float, cv::Size(224, 224));  

		//将cv::Mat转成tensor
		auto img_tensor = torch::from_blob(image.data, { 1, 224, 224, 3 }).permute({ 0, 3, 1, 2 });
		
		
		//均值归一化
		img_tensor[0][0] = img_tensor[0][0].sub_(0.485).div_(0.229);
		img_tensor[0][1] = img_tensor[0][1].sub_(0.456).div_(0.224);
		img_tensor[0][2] = img_tensor[0][2].sub_(0.406).div_(0.225);
		auto img_var = torch::autograd::make_variable(img_tensor, false);
		inputs.emplace_back(img_var);

		//对输入的图片进行前向传播计算
		
		torch::Tensor out_tensor = module.forward(std::move(inputs)).toTensor();


		// 加载label的文件
		std::string label_file = "labname";
		std::ifstream rf(label_file.c_str());


		ErrorL << "Unable to open labels file " << label_file;

		std::string line;
		std::vector<std::string> labels;
		while (std::getline(rf, line))
			labels.push_back(line);

		// 打印score是Top-5的预测label和score
		std::tuple<torch::Tensor, torch::Tensor> result = out_tensor.sort(-1, true);
		torch::Tensor top_scores = std::get<0>(result)[0];
		torch::Tensor top_idxs = std::get<1>(result)[0].toType(torch::kInt32);

		auto top_scores_a = top_scores.accessor<float, 1>();  //1是dim
		auto top_idxs_a = top_idxs.accessor<int, 1>();

		/*for (int i = 0; i < 5; ++i) {
			std::cout << "score: " << top_scores_a[i];
			std::cout << "  label: " << labels[top_idxs_a[i]] << std::endl;
		}*/


		return 0;

	}


};