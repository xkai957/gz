#pragma once
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <map>
#include <deque>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <memory>
#include <mutex>
#include <Util/util.h>
#include <SCL/SCL.hpp>

//namespace scl {
//	class config_file;
//};
namespace kmod {

	class KConfig :public toolkit::noncopyable
	{
		
		
	public:
		static KConfig &Instance() {
			static KConfig inst;
			return inst;
		}
		void setcfgfile( string name);
		void closecfgfile();
		scl::config_file* cfg() { return cfgfile; };
		KConfig() {};
		~KConfig() {};
	public:
		scl::config_file*  cfgfile;
		
	};
}
