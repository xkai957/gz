#pragma once

#include <string>
#include <iostream>
#include <Util/logger.h>
#include <Windows.h>
#include <NetSDK/HCNetSDK.h>
#include <time.h>
#include <NetSDK/plaympeg4.h>


class CAMX
{
public:
	CAMX();
	~CAMX();
	int realse();

	int connect();
	int disconnect();
	int gotopos(float x, float y, float z);
	int grab2img(std::string filename );
	int grab2img2(std::string filename);
	int grab2img3(char* imgbuf, long imgsize);
	int startvideo(HWND hwnd);
	int stopvideo();
	int capvideo(std::string filename);
	int stopcapvideo();

	int setpzt(float x, float y, float zoom);
	int getpzt(float &x, float &y, float &zoom);

	int geterr();

	int setip(std::string ip);
	int setuser(std::string user);
	int setpasswd(std::string passwd);

	int setposfromHeight_left(float higth, int zoom);
	int setposfromHeight_right(float higth, int zoom);
	//int setposfromHeight_zoom(float higth);
	int setposfromHeight_tail(float higth, int zoom);
	LONG getlUserID();
	LONG getlRealPlayHandle();

	

private:

	LONG		_lUserID;
	LONG		_lRealPlayHandle;
	std::string _ip;
	std::string _user;
	std::string _paswd;

	bool		_connected;
	bool		_isPlaying;
	bool		_IsRecording;

	float		_install_X;
	float		_install_Y;
	float		_install_Z;
public:
	long		_lPort;



};

