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


		//����һ���������
		std::vector<torch::jit::IValue> inputs;
		inputs.emplace_back(torch::rand({ 64, 3, 224, 224 }));
		// ��������һ��ǰ�򴫲�����Ҫʱ��
		auto t = (double)cv::getTickCount();
		torch::Tensor output = module.forward(std::move(inputs)).toTensor();
		t = (double)cv::getTickCount() - t;
		DebugL<<"execution time = "<< t / cv::getTickFrequency();
		inputs.pop_back();

		// ����һ��ͼƬ���ҽ��й�һ��
		cv::Mat image;
		image = cv::imread("imagename", 1);
		//ת��ΪRGB��ͨ��
		cv::cvtColor(image, image, cv::COLOR_BGR2RGB);  
		cv::Mat img_float;
		//���ȹ�һ����[0,1]����
		image.convertTo(img_float, CV_32F, 1.0 / 255);   
		//resize to 224��Ԥѵ����ģ��������batchsize x3 x 224 x 224
		cv::resize(img_float, img_float, cv::Size(224, 224));  

		//��cv::Matת��tensor
		auto img_tensor = torch::from_blob(image.data, { 1, 224, 224, 3 }).permute({ 0, 3, 1, 2 });
		
		
		//��ֵ��һ��
		img_tensor[0][0] = img_tensor[0][0].sub_(0.485).div_(0.229);
		img_tensor[0][1] = img_tensor[0][1].sub_(0.456).div_(0.224);
		img_tensor[0][2] = img_tensor[0][2].sub_(0.406).div_(0.225);
		auto img_var = torch::autograd::make_variable(img_tensor, false);
		inputs.emplace_back(img_var);

		//�������ͼƬ����ǰ�򴫲�����
		
		torch::Tensor out_tensor = module.forward(std::move(inputs)).toTensor();


		// ����label���ļ�
		std::string label_file = "labname";
		std::ifstream rf(label_file.c_str());


		ErrorL << "Unable to open labels file " << label_file;

		std::string line;
		std::vector<std::string> labels;
		while (std::getline(rf, line))
			labels.push_back(line);

		// ��ӡscore��Top-5��Ԥ��label��score
		std::tuple<torch::Tensor, torch::Tensor> result = out_tensor.sort(-1, true);
		torch::Tensor top_scores = std::get<0>(result)[0];
		torch::Tensor top_idxs = std::get<1>(result)[0].toType(torch::kInt32);

		auto top_scores_a = top_scores.accessor<float, 1>();  //1��dim
		auto top_idxs_a = top_idxs.accessor<int, 1>();

		/*for (int i = 0; i < 5; ++i) {
			std::cout << "score: " << top_scores_a[i];
			std::cout << "  label: " << labels[top_idxs_a[i]] << std::endl;
		}*/


		return 0;

	}


};