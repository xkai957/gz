#include "pch.h"
#include "CDetector.h"
#include <string>
#include <Util/logger.h>
#include "KConfig.h"

using namespace toolkit;
using namespace std;

namespace kmod 
{

	
	CDetectorX::CDetectorX()
	{

		/*init = NULL;
		detect_image = NULL;
		detect_mat = NULL;
		dispose = NULL;*/
		bvalid = false;


		/*CString str;
		GetModuleFileName(NULL, str.GetBuffer(255), 255);
		CString strpath = str.GetBuffer(0);
		int nposx = strpath.ReverseFind('\\') + 1;
		CString ss = strpath.Left(nposx) + _T("yolo_cpp_dll.dll");
		

		HINSTANCE hInst;
		hInst = LoadLibrary(ss);

		if (hInst == NULL)
		{
			return;
		}

		init = (INIT)GetProcAddress(hInst, "init");
		if (!init)
		{
			return;
		}

		detect_image = (DETECT_IMAGE)GetProcAddress(hInst, "detect_image");
		if (!detect_image)
		{
			return;
		}


		detect_mat = (DETECT_MAT)GetProcAddress(hInst, "detect_mat");
		if (!detect_mat)
		{
			return;
		}

		dispose = (DISPOSE)GetProcAddress(hInst, "dispose");
		if (!dispose)
		{
			return;
		}*/



	}
	CDetectorX::~CDetectorX()
	{

		dispose();

	}

	int  CDetectorX::InitializeYolo(const char* configurationFilename, const char* weightsFilename, int gpu)
	{
		if( init != NULL )
			return init(configurationFilename, weightsFilename, gpu);

		return -1;
	}

	void CDetectorX::Dispose()
	{
		dispose();
	}

	int CDetectorX::Detect(const char* filename, bbox_t_container &container)
	{
		return detect_image(filename, container);
	}

	int  CDetectorX::Detect(const unsigned char* data, const size_t data_length, bbox_t_container &container)
	{
		return detect_mat(data, data_length, container);
	}
}




