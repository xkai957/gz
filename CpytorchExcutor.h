#pragma once
#include <string>
namespace kmod
{
	class CpytorchExcutor
	{
	public:

		CpytorchExcutor();
		~CpytorchExcutor();

		int load_netparam(std::string mycfgpath);


	};


}

