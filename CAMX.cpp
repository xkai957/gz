#include "pch.h"
#include "CAMX.h"

#include <iostream>
#include <fstream>


#include "PlayM4.h"


using namespace toolkit;

typedef HWND(WINAPI *PROCGETCONSOLEWINDOW)();
PROCGETCONSOLEWINDOW GetConsoleWindowAPI;


void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
	char tempbuf[256] = { 0 };
	switch (dwType)
	{
	case EXCEPTION_RECONNECT:    //预览时重连
		break;
	default:
		break;
	}
}

CAMX::CAMX()
{
	_connected = false;
	_IsRecording = false;
	_isPlaying = false;
	_IsRecording = false;
	_lPort = -1;
	_lUserID = -1;
	_lRealPlayHandle = -1;


}
CAMX::~CAMX()
{

}

int CAMX::realse()
{

	disconnect();

	return 0;
}
void CALLBACK DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
//void CALLBACK  DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	long lFrameType = pFrameInfo->nType;
	if (lFrameType == T_AUDIO16)
	{

	}
	else if (lFrameType == T_YV12)
	{

	}
	else
	{

	}
}



void CALLBACK fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
	HWND hWnd = GetConsoleWindowAPI();
	CAMX * pcam = (CAMX *)pUser;

	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD: //系统头
		if (pcam->_lPort >= 0)
		{
			break;  //该通道取流之前已经获取到句柄，后续接口不需要再调用
		}
		if (!PlayM4_GetPort(&pcam->_lPort))  //获取播放库未使用的通道号
		{
			break;
		}

		if (dwBufSize > 0)
		{

			

			if (!PlayM4_SetStreamOpenMode(pcam->_lPort, STREAME_REALTIME))  //设置实时流播放模式
			{
				break;
			}

			if (!PlayM4_OpenStream(pcam->_lPort, pBuffer, dwBufSize, 1024 * 1024)) //打开流接口
			{
				break;
			}


			//设置解码回调函数 只解码不显示
	 		if (!PlayM4_SetDecCallBack(pcam->_lPort,DecCBFun))
	 		{
	 			//dRet=PlayM4_GetLastError(nPort);
	 			break;
	 		}

			////设置解码回调函数 解码且显示
			//if (!PlayM4_SetDecCallBackEx(_lPort, DecCBFun, NULL, NULL))
			//{
			//	//dRet = PlayM4_GetLastError(nPort);
			//	break;
			//}

			//if (!PlayM4_Play(_lPort, hWnd)) //播放开始
			if (!PlayM4_Play(pcam->_lPort, NULL)) //播放开始
			{
				break;
			}
		}
		break;
	case NET_DVR_STREAMDATA:   //码流数据
		if (dwBufSize > 0 && pcam->_lPort != -1)
		{
			if (!PlayM4_InputData(pcam->_lPort, pBuffer, dwBufSize))
			{
				break;
			}
		}
		break;
	default: //其他数据
		if (dwBufSize > 0 && pcam->_lPort != -1)
		{
			if (!PlayM4_InputData(pcam->_lPort, pBuffer, dwBufSize))
			{
				break;
			}
		}
		break;
	}
}

LONG CAMX::getlUserID()
{
	LONG userID = _lUserID;
	return userID;
}
LONG CAMX::getlRealPlayHandle()
{
	LONG RealPlayHandle = _lRealPlayHandle;
	return RealPlayHandle;
}
int CAMX::connect()
{

	HMODULE hKernel32 = GetModuleHandle(TEXT("kernel32"));
	GetConsoleWindowAPI = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32, "GetConsoleWindow");

	if (!_connected)
	{
		// 初始化
		NET_DVR_Init();
		//设置连接时间与重连时间
		NET_DVR_SetConnectTime(2000, 1);
		NET_DVR_SetReconnect(10000, true);
		//---------------------------------------
		//设置异常消息回调函数
		NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);
		//---------------------------------------
		// 获取控制台窗口句柄
		/*HMODULE hKernel32 = GetModuleHandle(TEXT("kernel32"));
		GetConsoleWindowAPI = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32, "GetConsoleWindow");*/
		//---------------------------------------
		// 注册设备
		//登录参数，包括设备地址、登录用户、密码等
		NET_DVR_USER_LOGIN_INFO struLoginInfo = { 0 };
		struLoginInfo.bUseAsynLogin = 0; //同步登录方式
		strcpy_s(struLoginInfo.sDeviceAddress, _ip.c_str()); //设备IP地址
		struLoginInfo.wPort = 8000; //设备服务端口
		strcpy_s(struLoginInfo.sUserName, _user.c_str()); //设备登录用户名
		strcpy_s(struLoginInfo.sPassword, _paswd.c_str()); //设备登录密码
		//设备信息, 输出参数
		NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = { 0 };
		_lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
		if (_lUserID < 0)
		{
			ErrorL << "Login failed, error code:" <<NET_DVR_GetLastError();
			NET_DVR_Cleanup();
			return -1;
		}
	
		_connected = true;
	}

	return 0;
}

int CAMX::disconnect()
{
	if (!_connected) return 0;

	if (_isPlaying)
	{
		stopcapvideo();
		stopvideo();
		return -1;
	}
	NET_DVR_Logout(_lUserID);
	
	_lUserID = -1;
	_connected = FALSE;
	return 0;
}
int CAMX::gotopos(float x, float y, float z)
{

	return 0;
}
int CAMX::grab2img(std::string filename)
{

	if (_lRealPlayHandle < 0)
	{
		ErrorL << "NET_DVR_RealPlay_V40 error\n";
		
		return -1;
	}
	else
	{
		const char* myname = filename.c_str();

		NET_DVR_SetCapturePictureMode(JPEG_MODE);
		if (NET_DVR_CapturePicture(_lRealPlayHandle, (char*)myname) )
		{
			return 0;
		}
		else
		{
			InfoL << "NET_DVR_CapturePicture error\n";
			return -2;
		}

	}

	return 0;
}
int CAMX::grab2img2(std::string filename)
{
	if (_lUserID < 0)
	{
		ErrorL << "NET_DVR_RealPlay_V40 error\n";
		return -1;
	}
	else
	{
		const char* myname = filename.c_str();

		NET_DVR_JPEGPARA JpegPara = { 0xff,0};

		//定义JPEG图像质量
		/*LPNET_DVR_JPEGPARA pJpgPara = new NET_DVR_JPEGPARA;
		pJpgPara->wPicQuality = 0;
		pJpgPara->wPicSize = 5;*/

/*
wPicSize
			图片尺寸：0 - CIF(352 * 288 / 352 * 240)，1 - QCIF(176 * 144 / 176 * 120)，
			2 - 4CIF(704 * 576 / 704 * 480)或D1(720 * 576 / 720 * 486)，3 - UXGA(1600 * 1200)， 
			4 - SVGA(800 * 600)，5 - HD720P(1280 * 720)，6 - VGA(640 * 480)，7 - XVGA(1280 * 960)，
			8 - HD900P(1600 * 900)，9 - HD1080P(1920 * 1080)，10 - 2560 * 1920， 11 - 1600 * 304，
			12 - 2048 * 1536，13 - 2448 * 2048，14 - 2448 * 1200，15 - 2448 * 800，16 - XGA(1024 * 768)，
			17 - SXGA(1280 * 1024)，18 - WD1(960 * 576 / 960 * 480), 19 - 1080I(1920 * 1080)，20 - 576 * 576，
			21 - 1536 * 1536，22 - 1920 * 1920，23 - 320 * 240，24 - 720 * 720，25 - 1024 * 768，
			26 - 1280 * 1280，27 - 1600 * 600， 28 - 2048 * 768，29 - 160 * 120，75 - 336 * 256，
			78 - 384 * 256，79 - 384 * 216，80 - 320 * 256，82 - 320 * 192，83 - 512 * 384，127 - 480 * 272，
			128 - 512 * 272， 161 - 288 * 320，162 - 144 * 176，163 - 480 * 640，164 - 240 * 320，165 - 120 * 160，
			166 - 576 * 720，167 - 720 * 1280，168 - 576 * 960，180 - 180 * 240, 181 - 360 * 480, 182 - 540 * 720,
			183 - 720 * 960, 184 - 960 * 1280, 185 - 1080 * 1440, 500 - 384 * 288, 0xff - Auto(使用当前码流分辨率)
wPicQuality
			图片质量系数：0 - 最好，1 - 较好，2 - 一般
*/
		if (NET_DVR_CaptureJPEGPicture(_lUserID, 1, &JpegPara,(char*)myname))
		{
			//
			//delete[] pJpgPara;
			return 0;
		}
		else
		{
			InfoL << "device capture error\n";
			//delete[] pJpgPara;
			return -2;
		}

	}

	return 0;
}

int CAMX::grab2img3( char* imgbuf, long imgsize)
{

	if (_lUserID < 0)
	{
		ErrorL << "NET_DVR_RealPlay_V40 error\n";
		return -1;
	}
	else
	{
		

		NET_DVR_JPEGPARA JpegPara = { 5,0 };

		JpegPara.wPicQuality = 0;
		JpegPara.wPicSize = 5;



		/*
		wPicSize
					图片尺寸：0 - CIF(352 * 288 / 352 * 240)，1 - QCIF(176 * 144 / 176 * 120)，
					2 - 4CIF(704 * 576 / 704 * 480)或D1(720 * 576 / 720 * 486)，3 - UXGA(1600 * 1200)，
					4 - SVGA(800 * 600)，5 - HD720P(1280 * 720)，6 - VGA(640 * 480)，7 - XVGA(1280 * 960)，
					8 - HD900P(1600 * 900)，9 - HD1080P(1920 * 1080)，10 - 2560 * 1920， 11 - 1600 * 304，
					12 - 2048 * 1536，13 - 2448 * 2048，14 - 2448 * 1200，15 - 2448 * 800，16 - XGA(1024 * 768)，
					17 - SXGA(1280 * 1024)，18 - WD1(960 * 576 / 960 * 480), 19 - 1080I(1920 * 1080)，20 - 576 * 576，
					21 - 1536 * 1536，22 - 1920 * 1920，23 - 320 * 240，24 - 720 * 720，25 - 1024 * 768，
					26 - 1280 * 1280，27 - 1600 * 600， 28 - 2048 * 768，29 - 160 * 120，75 - 336 * 256，
					78 - 384 * 256，79 - 384 * 216，80 - 320 * 256，82 - 320 * 192，83 - 512 * 384，127 - 480 * 272，
					128 - 512 * 272， 161 - 288 * 320，162 - 144 * 176，163 - 480 * 640，164 - 240 * 320，165 - 120 * 160，
					166 - 576 * 720，167 - 720 * 1280，168 - 576 * 960，180 - 180 * 240, 181 - 360 * 480, 182 - 540 * 720,
					183 - 720 * 960, 184 - 960 * 1280, 185 - 1080 * 1440, 500 - 384 * 288, 0xff - Auto(使用当前码流分辨率)
		wPicQuality
					图片质量系数：0 - 最好，1 - 较好，2 - 一般
		*/
		DWORD retlen = 0;
		if (NET_DVR_CaptureJPEGPicture_NEW(_lUserID, 1, &JpegPara, imgbuf,imgsize, &retlen))
		{			
			return retlen;
		}
		else
		{
			InfoL << "NET_DVR_CaptureJPEGPicture_NEW error\n";
			return -2;
		}

	}

	return 0;
}
int CAMX::startvideo( HWND hwnd)
{
	if (!_connected)
	{
		return -1;
	}

	if (!_isPlaying)
	{
		//启动预览并设置回调数据流
		NET_DVR_PREVIEWINFO struPlayInfo = { 0 };
		struPlayInfo.hPlayWnd = hwnd;		// GetDlgItem(IDC_STATIC_PLAY1)->m_hWnd;    //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
		struPlayInfo.lChannel = 1;			//预览通道号
		struPlayInfo.dwStreamType = 0;       //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
		struPlayInfo.dwLinkMode = 1;       //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
		struPlayInfo.bBlocked = 1;       //0- 非阻塞取流，1- 阻塞取流
		
		_lRealPlayHandle = NET_DVR_RealPlay_V40(_lUserID, &struPlayInfo, NULL, NULL);
		//_lRealPlayHandle = NET_DVR_RealPlay_V40(_lUserID, &struPlayInfo, fRealDataCallBack, this);

		if (_lRealPlayHandle < 0)
		{
			ErrorL << "NET_DVR_RealPlay_V40 error";			
			//NET_DVR_Logout(_lUserID);
			//NET_DVR_Cleanup();
			_connected = false;
			return -2;
		}
		_isPlaying = TRUE;
		
	}
	
	return 0;
}
int CAMX::stopvideo()
{
	if (_isPlaying)
	{
		NET_DVR_StopRealPlay(_lRealPlayHandle);
		_lRealPlayHandle = -1;
		_isPlaying = false;
	}

	return 0;
}
int CAMX::capvideo(std::string filename)
{
	if (!_IsRecording)
	{
		if (!NET_DVR_SaveRealData(_lRealPlayHandle, (char*)filename.c_str()))
		{
			return -1;
		}
		_IsRecording = true;
	}
	return 0;
}

int CAMX::stopcapvideo()
{
	if (_IsRecording) 
	{
		if (!NET_DVR_StopSaveRealData(_lRealPlayHandle))
		{
			return -1;
		}
		_IsRecording = false;
	}
	
	return 0;
}

int CAMX::setpzt(float x, float y, float zoom)
{

	NET_DVR_PTZPOS pos;

	pos.wPanPos = x;
	pos.wTiltPos = y;
	pos.wZoomPos = zoom;
	pos.wAction = 1;

	if (_connected)
	{
		if (NET_DVR_SetDVRConfig(_lUserID, NET_DVR_SET_PTZPOS, 0xffffffff, &pos, sizeof(NET_DVR_PTZPOS)))
		{
			return 0;
		}
	}
	return -1;
}
int CAMX::getpzt(float &x, float &y, float &zoom)
{

	NET_DVR_PTZPOS pos;

	if (_connected)
	{
		DWORD len=0;
		if (NET_DVR_GetDVRConfig(_lUserID, NET_DVR_GET_PTZPOS, 0xffffffff, &pos, sizeof(NET_DVR_PTZPOS),&len))
		{
			x = pos.wPanPos;
			y = pos.wTiltPos;
			zoom = pos.wZoomPos;
			return 0;
		}
		else
		{
			char* msg = NET_DVR_GetErrorMsg();
			ErrorL << msg;
		}

	}

	return -1;
}

int  CAMX::geterr()
{

	return 0;
}


int  CAMX::setip(std::string ip)
{
	_ip = ip;
	return 0;
}

int  CAMX::setuser(std::string user)
{
	_user = user;

	return 0;
}

int  CAMX::setpasswd(std::string passwd)
{
	_paswd = passwd;
	return 0;
}

int  CAMX::setposfromHeight_left(float h, int zoom)
{
	// 一号行车 192.168.1.197
	NET_DVR_PTZPOS pos;
	pos.wAction = 1;
	pos.wPanPos = 4360;
	pos.wTiltPos = 2080 - 0.018 * (h - 1330);
	pos.wZoomPos = zoom;
	if (_connected)
	{
		if (NET_DVR_SetDVRConfig(_lUserID, NET_DVR_SET_PTZPOS, 0xffffffff, &pos, sizeof(NET_DVR_PTZPOS)))
		{
			return 0;
		}
	}
	return -1;

	


	// 二号行车 192.168.1.198
	/*NET_DVR_PTZPOS pos;
	pos.wAction = 1;
	pos.wPanPos = 402;
	pos.wTiltPos = 1910 - 0.0088 * (h - 1300);
	pos.wZoomPos = zoom;
	if (_connected)
	{
		if (NET_DVR_SetDVRConfig(_lUserID, NET_DVR_SET_PTZPOS, 0xffffffff, &pos, sizeof(NET_DVR_PTZPOS)))
		{
			return 0;
		}
	}
	return -1;*/
}

int CAMX::setposfromHeight_right(float h, int zoom)
{

	// 一号行车 192.168.1.197
	NET_DVR_PTZPOS pos;
	pos.wAction = 1;
	pos.wPanPos = 9337;
	pos.wTiltPos = 1910 - 0.00784 * (h - 1330);
	pos.wZoomPos = zoom;
	if (_connected)
	{
		if (NET_DVR_SetDVRConfig(_lUserID, NET_DVR_SET_PTZPOS, 0xffffffff, &pos, sizeof(NET_DVR_PTZPOS)))
		{
			return 0;
		}
	}
	return -1;


	// 二号行车 192.168.1.198
	/*NET_DVR_PTZPOS pos;
	pos.wAction = 1;
	pos.wPanPos = 5921;
	pos.wTiltPos = 2050 - 0.022 * (h - 1300);
	pos.wZoomPos = zoom;
	if (_connected)
	{
		if (NET_DVR_SetDVRConfig(_lUserID, NET_DVR_SET_PTZPOS, 0xffffffff, &pos, sizeof(NET_DVR_PTZPOS)))
		{
			return 0;
		}
	}
	return -1;*/
}

//int CAMX::setposfromHeight_zoom(float h)
//{
//	NET_DVR_PTZPOS pos;
//
//	pos.wAction = 1;
//
//	pos.wPanPos = 5921;
//	pos.wTiltPos = 0.017 * (h - 5800) + 1872;
//	pos.wZoomPos = 32;
//
//	if (_connected)
//	{
//		if (NET_DVR_SetDVRConfig(_lUserID, NET_DVR_SET_PTZPOS, 0xffffffff, &pos, sizeof(NET_DVR_PTZPOS)))
//		{
//			return 0;
//		}
//	}
//
//	return -1;
//}

int CAMX::setposfromHeight_tail(float h, int zoom)
{

	// 一号行车 192.168.1.197
	NET_DVR_PTZPOS pos;
	pos.wAction = 1;
	pos.wPanPos = 9816;
	pos.wTiltPos = 1911;
	pos.wZoomPos = zoom;
	if (_connected)
	{
		if (NET_DVR_SetDVRConfig(_lUserID, NET_DVR_SET_PTZPOS, 0xffffffff, &pos, sizeof(NET_DVR_PTZPOS)))
		{
			return 0;
		}
	}
	return -1;





	// 二号行车	192.168.1.198
	/*NET_DVR_PTZPOS pos;
	pos.wAction = 1;
	pos.wPanPos = 1077;
	pos.wTiltPos = 1289;
	pos.wZoomPos = zoom;
	if (_connected)
	{
		if (NET_DVR_SetDVRConfig(_lUserID, NET_DVR_SET_PTZPOS, 0xffffffff, &pos, sizeof(NET_DVR_PTZPOS)))
		{
			return 0;
		}
	}
	return -1;*/
}

