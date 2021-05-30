#include "KConfig.h"

#include <SCL/SCL.hpp>

namespace kmod {

	void KConfig::setcfgfile(string name)
	{
		cfgfile = new scl::config_file(name, scl::config_file::READ);
	}

	void KConfig::closecfgfile()
	{

		if (cfgfile != nullptr)
		{
			cfgfile->close();
			cfgfile = nullptr;
		}
	}
}
