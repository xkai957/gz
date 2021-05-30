
// KOCDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "KOC.h"
#include "KOCDlg.h"
#include "afxdialogex.h"
#include "KConfig.h"

#include <stdio.h>
#include <io.h>

#include <Util/logger.h>
#include <string>
#include <vector>
#include <iostream>
#include "cstdlib"
#include "windows.h"

#include <rapidjson/document.h>     
#include <rapidjson/prettywriter.h> 
#include <rapidjson/rapidjson.h>

#include <Network/TcpServer.h>
#include <Network/TcpSession.h>
#include "KTCPServer.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<opencv2/stitching.hpp>



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using namespace toolkit;
using namespace std;
using namespace rapidjson;
using namespace kmod;
using namespace cv;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CKOCDlg 对话框



CKOCDlg::CKOCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_KOC_DIALOG, pParent)
	, _pospan(0)
	, _postilt(0)
	, _poszoom(0)
	, m_strInfo(_T(""))
	

{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	for (int i = 0; i < MAX_CAMSUM; i++)
	{
		_pCAM[i] = NULL;
		_camcheck[i] = TRUE;
	}

	_pComm = NULL;

	_isbusy = false;
}

void CKOCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PAN, _pospan);
	DDX_Text(pDX, IDC_EDIT_TILT, _postilt);
	DDX_Text(pDX, IDC_EDIT_ZOOM, _poszoom);
	DDX_Text(pDX, IDC_EDIT_INFO, m_strInfo);

	DDX_Check(pDX, IDC_CHECK_LEFT, _camcheck[0]);
	DDX_Check(pDX, IDC_CHECK_RIGHT, _camcheck[1]);
	DDX_Check(pDX, IDC_CHECK_TAIL, _camcheck[2]);
	DDX_Text(pDX, IDC_TXT_LEFTCAMINFO, _caminfo[0]);
	DDX_Text(pDX, IDC_TXT_RIGHTCAMINFO, _caminfo[1]);
	DDX_Text(pDX, IDC_TXT_TAILCAMINFO, _caminfo[2]);
}

BEGIN_MESSAGE_MAP(CKOCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHECK_LEFT, &CKOCDlg::OnBnClickedCheckLeft)
	ON_BN_CLICKED(IDC_CHECK_RIGHT, &CKOCDlg::OnBnClickedCheckRight)
	ON_BN_CLICKED(IDC_CHECK_TAIL, &CKOCDlg::OnBnClickedCheckTail)
	ON_BN_CLICKED(IDC_BUTTON_CAPVID, &CKOCDlg::OnBnClickedButtonCapvid)
	ON_BN_CLICKED(IDC_BUTTON_GO, &CKOCDlg::OnBnClickedButtonGo)
	ON_BN_CLICKED(IDC_BUTTON_CAPIMG, &CKOCDlg::OnBnClickedButtonCapimg)
	ON_BN_CLICKED(IDC_BUTTON_INFOCLEAR, &CKOCDlg::OnBnClickedButtonInfoclear)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_GETPOS, &CKOCDlg::OnBnClickedButtonGetpos)
	ON_BN_CLICKED(IDC_BUTTON_STOPCAPVID, &CKOCDlg::OnBnClickedButtonStopcapvid)
	ON_BN_CLICKED(IDC_BUTTON_TESTCOMM, &CKOCDlg::OnBnClickedButtonTestcomm)
	ON_BN_CLICKED(IDC_BUTTON_TESTLR, &CKOCDlg::OnBnClickedButtonTestlr)
	ON_BN_CLICKED(IDC_BTN_TESTYOLO, &CKOCDlg::OnBnClickedBtnTestyolo)
END_MESSAGE_MAP()


// CKOCDlg 消息处理程序

BOOL CKOCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	for (int i = 0; i < MAX_CAMSUM; i++)
	{
		_pCAM[i] = new CAMX();
	}


	_pCAM[0]->setip(kmod::KConfig::Instance().cfg()->get<string>("leftip"));
	_pCAM[0]->setuser(kmod::KConfig::Instance().cfg()->get<string>("leftuser"));
	_pCAM[0]->setpasswd(kmod::KConfig::Instance().cfg()->get<string>("leftpassword"));

	_pCAM[1]->setip(kmod::KConfig::Instance().cfg()->get<string>("rightip"));
	_pCAM[1]->setuser(kmod::KConfig::Instance().cfg()->get<string>("rightuser"));
	_pCAM[1]->setpasswd(kmod::KConfig::Instance().cfg()->get<string>("rightpassword"));


	_pCAM[2]->setip(kmod::KConfig::Instance().cfg()->get<string>("tailip"));
	_pCAM[2]->setuser(kmod::KConfig::Instance().cfg()->get<string>("tailuser"));
	_pCAM[2]->setpasswd(kmod::KConfig::Instance().cfg()->get<string>("tailpassword"));


	for (int i = 0; i < MAX_CAMSUM; i++)
	{
		_pCAM[i]->connect();
	}

	_pCAM[0]->startvideo(GetDlgItem(IDC_STATIC_CAMLEFT)->m_hWnd);
	_pCAM[1]->startvideo(GetDlgItem(IDC_STATIC_CAMRIGHT)->m_hWnd);
	_pCAM[2]->startvideo(GetDlgItem(IDC_STATIC_CAMTAIL)->m_hWnd);


	for (int i = 0; i < 3; i++)
	{
		_box[i] = KTCPClientBox::Ptr(new KTCPClientBox);
		//box[i]->setOnErr();
		//sendBoxCMD(i, 0);//连接后，关闭箱子
		Sleep(100);
	}


	InfoL << "left摄像头ip:" << kmod::KConfig::Instance().cfg()->get<string>("leftip") << endl;
	InfoL << "right摄像头ip:" << kmod::KConfig::Instance().cfg()->get<string>("rightip") << endl;
	InfoL << "tail摄像头ip:" << kmod::KConfig::Instance().cfg()->get<string>("tailip") << endl;



	//UDP 通讯初始化
	_pComm = get_defaultCOMM();
	_pComm->init();

	/*Socket::Ptr sock;
	_server = KTCPServer::Ptr(new KTCPServer(sock));
	_server->init();
	InfoL << "开启监听tcp端口" << endl;*/

	int myserverport = kmod::KConfig::Instance().cfg()->get<int>("mytcpserver_port");

	_cmdServer = TcpServer::Ptr(new TcpServer());
	_cmdServer->start<kmod::KTCPServer>(myserverport);
	InfoL << "开始监听9001端口" << endl;
	

	_caminfo[0] = "pos:0,0,0";
	_caminfo[1] = "pos:0,0,0";
	_caminfo[2] = "pos:0,0,0";

	UpdateData(FALSE);


	CString str;
	GetModuleFileName(NULL, str.GetBuffer(255), 255);
	USES_CONVERSION;//宏
	string myexepath = W2A(str.GetBuffer(0));
	int npos = myexepath.rfind('\\') + 1;
	myexepath.erase(npos, myexepath.size() - npos);

	netcfg = myexepath + "cfg\\" + kmod::KConfig::Instance().cfg()->get<string>("netname");
	wgscfg = myexepath + "cfg\\" + kmod::KConfig::Instance().cfg()->get<string>("weightsname");

	bbox_t_container boxt;
	int ret = init(netcfg.c_str(), wgscfg.c_str(), 0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CKOCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CKOCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CKOCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CKOCDlg::OnBnClickedCheckLeft()
{
	// TODO: 在此添加控件通知处理程序代码
	if (_camcheck[0])
	{
		_camcheck[0] = FALSE;
	}
	else if (!_camcheck[0])
	{
		_camcheck[0] = TRUE;
	}
}


void CKOCDlg::OnBnClickedCheckRight()
{
	// TODO: 在此添加控件通知处理程序代码
	if (_camcheck[1])
	{
		_camcheck[1] = FALSE;
	}
	else if (!_camcheck[1])
	{
		_camcheck[1] = TRUE;
	}
}


void CKOCDlg::OnBnClickedCheckTail()
{
	// TODO: 在此添加控件通知处理程序代码
	if (_camcheck[2])
	{
		_camcheck[2] = FALSE;
	}
	else if (!_camcheck[2])
	{
		_camcheck[2] = TRUE;
	}
}


BOOL CKOCDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	// TODO: 在此添加专用代码和/或调用基类

	return CDialogEx::Create(lpszTemplateName, pParentWnd);
}


LRESULT CKOCDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (message == UDP_MSG)
	{
		CString   strddd = (LPCTSTR)lParam;   
		AddInfoText(strddd);
		UpdateData(FALSE);

		if (_isbusy)
		{
			_pComm->sendCMD("busy!");
		}
		else
		{
			
			InfoL << "开始处理udp消息" << endl;
			ParseCommand(strddd);
			InfoL << "结束处理udp消息" << endl;
			
		}
	}
	else if (message == TCP_MSG)
	{
		CString   strddd = (LPCTSTR)lParam;
		AddInfoText(strddd);
		UpdateData(FALSE);


		if (_isbusy)
		{
			_pComm->sendCMD("busy!");

		}
		else
		{
			InfoL << "开始处理TCP消息" << endl;
			ParseCommand(strddd);
			InfoL << "TCP消息处理结束" << endl;
		}
	}
	return CDialogEx::WindowProc(message, wParam, lParam);
}


void CKOCDlg::PostNcDestroy()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDialogEx::PostNcDestroy();
}


BOOL CKOCDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此添加专用代码和/或调用基类

	return CDialogEx::PreCreateWindow(cs);
}




BOOL CKOCDlg::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	KillTimer(1);
	for (int i = 0; i < MAX_CAMSUM; i++)
	{
		if (_pCAM[i] != NULL) 
		{
			_pCAM[i]->realse();
			delete _pCAM[i];
			_pCAM[i] = NULL;
		}
	}
	for (int i = 0; i < 3; i++)
	{
		_box[i]->shutdown();
	}

	return CDialogEx::DestroyWindow();
}


void CKOCDlg::OnBnClickedButtonGo()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	CString info;

	info.Format( _T("pan=%.2f,tile=%.2f,zoom=%.2f\r\n"), _pospan, _postilt, _poszoom);

	m_strInfo += "go ";
	m_strInfo += info;

	UpdateData(FALSE);

	for (int i = 0; i < MAX_CAMSUM; i++)
	{
		if (_camcheck[i])
		{
			_pCAM[i]->setpzt(_pospan, _postilt, _poszoom);
		}
	}
}

int 	 CKOCDlg::GetLRImage(int event)
{

	cv::Mat combine;
	cv::Mat	a, b;
	if (!LeftPic_dect.empty())
	{
		InfoL << "LeftPic_dect:" << LeftPic_dect << endl;
		a = cv::imread(LeftPic_dect);
	}
	else
	{
		InfoL << "LeftPic_nodect:" << LeftPic_nodect << endl;
		a = cv::imread(LeftPic_nodect);
	}

	if (!RightPic_dect.empty())
	{
		InfoL << "RightPic_dect:" << RightPic_dect << endl;
		b = cv::imread(RightPic_dect);
	}
	else
	{
		InfoL << "RightPic_nodect:" << RightPic_nodect << endl;
		b = cv::imread(RightPic_nodect);
	}

	resize(a, a, Size(960, 540), 0, 0, INTER_LINEAR);
	resize(b, b, Size(960, 540), 0, 0, INTER_LINEAR);

	//水平拼接
	hconcat(a, b, combine);
	//垂直拼接
	//vconcat(a, b, combine);
	string webpath = kmod::KConfig::Instance().cfg()->get<string>("webpath");
	char Img_Path[MAX_PATH];
	if (event == 1004) {
		sprintf(Img_Path, "%s/%s.jpg", webpath.c_str(), "lr-1004");
	}
	else if(event == 1003){
		sprintf(Img_Path, "%s/%s.jpg", webpath.c_str(), "lr-1003");
	}
	else {
		sprintf(Img_Path, "%s/%s.jpg", webpath.c_str(), "lr-1005");
	}
	
	vector<int> compression_params;
	compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);  //选择jpeg
	compression_params.push_back(50); //在这个填入你要的图片质量
	imwrite(Img_Path, combine, compression_params);

	return 0;
	//string savepath = kmod::KConfig::Instance().cfg()->get<string>("savepath");
	//CTime CurTime = CTime::GetCurrentTime();
	//char PicName[MAX_PATH];
	//sprintf(PicName, "%s/%04d%02d%02d%02d%02d%02d_LR.jpg", savepath.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
	//	CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
	//cv::Mat imgsrc[2];
	//int len = 1280 * 1280 * 2;
	//char * pdat = new char[len];
	//for (int i = 0; i < 2; i++)
	//{
	//	int retlen = _pCAM[i]->grab2img3(pdat, len);
	//	if (retlen > 0)
	//	{
	//		vector<char>vecData;
	//		vecData.insert(vecData.end(), pdat, pdat + retlen);
	//		imgsrc[i] = cv::imdecode(cv::Mat(vecData), 1);
	//	}
	//}
	//delete[] pdat;
	//if (!imgsrc[0].empty() && !imgsrc[1].empty())
	//{
	//	cv::Mat desimg = cv::Mat(imgsrc[0].rows, imgsrc[0].cols*2, imgsrc[0].type());
	//	cv::Mat img0 = desimg(cv::Range(0, desimg.rows), cv::Range(0, imgsrc[0].cols));
	//	cv::Mat img1 = desimg(cv::Range(0, desimg.rows), cv::Range(imgsrc[0].cols, 2*imgsrc[0].cols));
	//	imgsrc[0].copyTo(img0);
	//	imgsrc[1].copyTo(img1);
	//	vector<int> compression_params;
	//	compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);  //选择jpeg
	//	compression_params.push_back(50); //在这个填入你要的图片质量
	//	cv::imwrite(PicName, desimg, compression_params);
	//	return 0;
	//}

}

int CKOCDlg::GetImage(int id, int event)
{
	cv::Mat	a;
	if (id == 0 && !SlagPic.empty())
	{
		InfoL << "SlagPic:" << SlagPic << endl;
		a = cv::imread(SlagPic);
	}
	else if(id == 1 && !TailPic.empty())
	{
		InfoL << "TailPic:" << TailPic << endl;
		a = cv::imread(TailPic);
	}

	
	string webpath = kmod::KConfig::Instance().cfg()->get<string>("webpath");
	char Img_Path[MAX_PATH];
	if (id == 0)
	{
		if (event == 1001) {
			sprintf(Img_Path, "%s/%s.jpg", webpath.c_str(), "slag-1001");
		}
		
	}
	else if (id == 1)
	{
		if (event == 1007) {
			sprintf(Img_Path, "%s/%s.jpg", webpath.c_str(), "tail-1007");
		}
		else if(event == 1006){
			sprintf(Img_Path, "%s/%s.jpg", webpath.c_str(), "tail-1006");
		}
		else {
			sprintf(Img_Path, "%s/%s.jpg", webpath.c_str(), "tail-1008");
		}
		
	}
	
	vector<int> compression_params;
	compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);  //选择jpeg
	compression_params.push_back(50); //在这个填入你要的图片质量
	imwrite(Img_Path, a, compression_params);

	return 0;
}

int CKOCDlg::GoToHight_left(float h, int zoom)
{
	if (_camcheck[0])
	{
		_pCAM[0]->setposfromHeight_left(h, zoom);
	}
	return 0;
}
int CKOCDlg::GoToHight_right(float h, int zoom)
{
	if (_camcheck[1])
	{
		_pCAM[1]->setposfromHeight_right(h, zoom);
	}
	return 0;
}
int CKOCDlg::GoToHight_tail(float h, int zoom)
{
	if (_camcheck[2])
	{
		_pCAM[2]->setposfromHeight_tail(h, zoom);
	}
	return 0;
}


void CKOCDlg::OnBnClickedButtonCapimg_tail_nodect()
{

	string savepath_tail = kmod::KConfig::Instance().cfg()->get<string>("savepath_tail");

	CTime CurTime = CTime::GetCurrentTime();
	UpdateData(TRUE);

	if (_camcheck[2])
	{
		char PicName[MAX_PATH];

		sprintf(PicName, "%s/%04d%02d%02d%02d%02d%02d_ch%02d.jpg", savepath_tail.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
			CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), 2);

		_pCAM[2]->grab2img2(PicName);

		TailPic = PicName;
		InfoL << "TailPic" << TailPic << endl;
	}
}

void CKOCDlg::OnBnClickedButtonCapimg_tail(int id)
{

	string savepath_tail = kmod::KConfig::Instance().cfg()->get<string>("savepath_tail");

	CTime CurTime = CTime::GetCurrentTime();
	UpdateData(TRUE);

	bbox_t_container boxt;
	_class_id_tail = -1;

	if (_camcheck[2])
	{
		char PicName[MAX_PATH];

		sprintf(PicName, "%s/%04d%02d%02d%02d%02d%02d_ch%02d.jpg", savepath_tail.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
			CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), 2);

		_pCAM[2]->grab2img2(PicName);

		if (id == 0)
		{
			SlagPic = PicName;
			InfoL << "SlagPic" << SlagPic << endl;
		}
		else if (id == 1)
		{
			TailPic = PicName;
			InfoL << "TailPic" << TailPic << endl;
		}


		int ret = detect_image(PicName, boxt);

		if (ret > 0)
		{
			cv::Mat img = cv::imread(PicName);
			for (int j = 0; j < ret; j++)
			{
				cv::Scalar color(0, 255, 0);
				cv::rectangle(img, cv::Rect(boxt.candidates[j].x, boxt.candidates[j].y,
					boxt.candidates[j].w, boxt.candidates[j].h), color, 2);
				std::string clsid = std::to_string(boxt.candidates[j].obj_id);
				cv::putText(img, clsid, cv::Point2f(boxt.candidates[j].x, boxt.candidates[j].y - 16),
					cv::FONT_HERSHEY_COMPLEX_SMALL, 2, cv::Scalar(0, 0, 255), 2);

				_class_id_tail = boxt.candidates[j].obj_id;

				string dectpath = kmod::KConfig::Instance().cfg()->get<string>("dectpath");
				char DectPicName[MAX_PATH];

				sprintf(DectPicName, "%s/%04d%02d%02d%02d%02d%02d_ch%02d.jpg", dectpath.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
					CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), 2);

				cv::imwrite(DectPicName, img);
			}
		}
		else
		{
			string failepath = kmod::KConfig::Instance().cfg()->get<string>("failepath");
			char FailePicName[MAX_PATH];
			sprintf(FailePicName, "%s/%04d%02d%02d%02d%02d%02d_ch%02d.jpg", failepath.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
				CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), 2);
			MoveFileA(PicName, FailePicName);
			if (id == 0)
			{
				SlagPic = FailePicName;
				InfoL << "SlagPic" << SlagPic << endl;
			}
			else if (id == 1)
			{
				TailPic = FailePicName;
				InfoL << "TailPic" << TailPic << endl;
			}
		}
	}
}

void CKOCDlg::OnBnClickedButtonCapimg_nodect()
{
	string savepath = kmod::KConfig::Instance().cfg()->get<string>("savepath");

	CTime CurTime = CTime::GetCurrentTime();
	UpdateData(TRUE);

	for (int i = 0; i < MAX_CAMSUM - 1; i++)
	{
		if (_camcheck[i])
		{
			char PicName[MAX_PATH];

			sprintf(PicName, "%s/%04d%02d%02d%02d%02d%02d_ch%02d.jpg", savepath.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
				CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), i);

			_pCAM[i]->grab2img2(PicName);

			if (i == 0)
			{
				LeftPic_dect = PicName;
				InfoL << "LeftPic_dect:" << LeftPic_dect << endl;
			}
			else if (i == 1)
			{
				RightPic_dect = PicName;
				InfoL << "RightPic_dect:" << RightPic_dect << endl;
			}
		}
	}
}


void CKOCDlg::OnBnClickedButtonCapimg()
{
	// TODO: 在此添加控件通知处理程序代码
	
	string savepath = kmod::KConfig::Instance().cfg()->get<string>("savepath");

	CTime CurTime = CTime::GetCurrentTime();	
	UpdateData(TRUE);

	// hook_success 0
	// hook_faile 1
	// slag 2
	// tail_success 3
	// tail_fail 4

	bbox_t_container boxt;
	_class_id_left = -1;
	_class_id_right = -1;
	/*_class_id_tail = -1;*/
	for (int i = 0; i < MAX_CAMSUM - 1; i++)
	{
		if (_camcheck[i])
		{
			char PicName[MAX_PATH];

			sprintf(PicName, "%s/%04d%02d%02d%02d%02d%02d_ch%02d.jpg", savepath.c_str(),CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
				CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), i);		

			_pCAM[i]->grab2img2(PicName);

			int ret = detect_image(PicName, boxt);
			
			if (ret > 0)
			{
				cv::Mat img = cv::imread(PicName);
				for (int j = 0; j < ret; j++)
				{
					cv::Scalar color(0, 255, 0);
					cv::rectangle(img, cv::Rect(boxt.candidates[j].x, boxt.candidates[j].y,
						boxt.candidates[j].w, boxt.candidates[j].h), color, 2);
					std::string clsid = std::to_string(boxt.candidates[j].obj_id);
					cv::putText(img, clsid, cv::Point2f(boxt.candidates[j].x, boxt.candidates[j].y - 16),
						cv::FONT_HERSHEY_COMPLEX_SMALL, 2, cv::Scalar(0, 0, 255), 2);

					string dectpath = kmod::KConfig::Instance().cfg()->get<string>("dectpath");
					char DectPicName[MAX_PATH];
					sprintf(DectPicName, "%s/%04d%02d%02d%02d%02d%02d_ch%02d.jpg", dectpath.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
						CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), i);

					if (i == 0)
					{
						LeftPic_dect = DectPicName;
						_class_id_left = boxt.candidates[j].obj_id;
						InfoL << "LeftPic_dect:" << LeftPic_dect << endl;
					}
					else if (i == 1)
					{
						RightPic_dect = DectPicName;
						_class_id_right = boxt.candidates[j].obj_id;
						InfoL << "RightPic_dect:" << RightPic_dect << endl;
					}

					cv::imwrite(DectPicName, img);
				}
			}
			else
			{
				string failepath = kmod::KConfig::Instance().cfg()->get<string>("failepath");
				char FailePicName[MAX_PATH];
				sprintf(FailePicName, "%s/%04d%02d%02d%02d%02d%02d_ch%02d.jpg", failepath.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
					CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), i);
				MoveFileA(PicName, FailePicName);
				if (i == 0)
				{
					LeftPic_nodect = FailePicName;
					InfoL << "LeftPic_nodect:" << LeftPic_nodect << endl;
				}
				else if (i == 1)
				{
					RightPic_nodect = FailePicName;
					InfoL << "RightPic_nodect:" << RightPic_nodect << endl;
				}
			}
		}

	}
}

void CKOCDlg::OnBnClickedButtonCapimg_collect()
{
	string savepath_left = kmod::KConfig::Instance().cfg()->get<string>("savepath_left");
	string savepath_right = kmod::KConfig::Instance().cfg()->get<string>("savepath_right");
	string savepath_tail = kmod::KConfig::Instance().cfg()->get<string>("savepath_tail");

	CTime CurTime = CTime::GetCurrentTime();
	UpdateData(TRUE);

	char DirName[MAX_PATH];
	sprintf(DirName, "%s\\%04d%02d%02d%02d", savepath_left.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetHour());
	string dir = DirName;
	if (_access(dir.c_str(), 0) == -1)
	{
		OnBnClickedButtonStopcapvid();
		string command;
		command = "mkdir " + dir;
		system(command.c_str());
		OnBnClickedButtonCapvid();
	}

	for (int i = 0; i < MAX_CAMSUM; i++)
	{
		if (_camcheck[i])
		{
			char PicName[MAX_PATH];

			if (i == 0)
			{
				sprintf(PicName, "%s\\%04d%02d%02d%02d%02d%02d_ch%02d.jpg", dir.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
					CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), i);
			}
			else if (i == 1)
			{
				sprintf(PicName, "%s\\%04d%02d%02d%02d%02d%02d_ch%02d.jpg", dir.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
					CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), i);
			}
			else
			{
				sprintf(PicName, "%s\\%04d%02d%02d%02d%02d%02d_ch%02d.jpg", dir.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
					CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), i);
			}

			_pCAM[i]->grab2img2(PicName);
			
		}

	}
}


void CKOCDlg::OnBnClickedButtonInfoclear()
{
	// TODO: 在此添加控件通知处理程序代码

	m_strInfo.Empty();
	UpdateData(FALSE);
}
void CKOCDlg::AddInfoText(CString  infotxt)
{
	m_strInfo += infotxt;	
	SetTimer(1, 100, NULL);

}


void CKOCDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	KillTimer(1);

	if (m_strInfo.GetLength() > 2500)
		m_strInfo.Empty();
	  
	
	UpdateData(FALSE);
	CDialogEx::OnTimer(nIDEvent);
}


void CKOCDlg::OnBnClickedButtonGetpos()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	for (int i = 0; i < MAX_CAMSUM; i++)
	{
		float x = 0, y = 0, z = 0;
		if (_pCAM[i] != NULL)
		{
			if (_pCAM[i]->getpzt(x, y, z) == 0)
			{
				CString info;
				info.Format(_T("pan=%.2f,tile=%.2f,zoom=%.2f\r\n"), x, y, z);
				_caminfo[i] = info;

			}
		}
		else
		{
			_caminfo[i] = "unconnected";

		}

	}
	UpdateData(FALSE);
}

void CKOCDlg::OnBnClickedButtonCapvid()
{
	// TODO: 在此添加控件通知处理程序代码
	string vidpath = kmod::KConfig::Instance().cfg()->get<string>("savevideopath");
	CTime CurTime = CTime::GetCurrentTime();
	UpdateData(TRUE);

	char DirVideoName[MAX_PATH];
	sprintf(DirVideoName, "%s\\%04d%02d%02d%02d", vidpath.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetHour());
	string dirVideo = DirVideoName;
	if (_access(dirVideo.c_str(), 0) == -1)
	{
		string command;
		command = "mkdir " + dirVideo;
		system(command.c_str());
	}

	for (int i = 0; i < MAX_CAMSUM; i++)
	{
		if (_camcheck[i])
		{
			char VidName[MAX_PATH];
			sprintf(VidName, "%s\\%04d%02d%02d%02d%02d%02d_ch%02d.mp4", dirVideo.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
				CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), i);

			_pCAM[i]->capvideo(VidName);

		}

	}

}
void CKOCDlg::OnBnClickedButtonStopcapvid()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	for (int i = 0; i < MAX_CAMSUM; i++)
	{
		if (_camcheck[i])
		{
			_pCAM[i]->stopcapvideo();
		}

	}
}


void CKOCDlg::OnBnClickedButtonTestcomm()
{
	// TODO: 在此添加控件通知处理程序代码
	for (int i = 0; i < 3; i++)
	{
		SendBoxCMD(i, 1);
	}
	//_pComm->sendCMD("testcmd");
}

void	CKOCDlg::OperationBox(int boxID, int nowStatus, int expectStatus, int finalLoop)
{
	// 打开尾钩防护柜的箱子
	for (int i = 0; i < finalLoop; i++)
	{
		if (GetBoxStatus(boxID) == nowStatus)	
		{
			SendBoxCMD(boxID, expectStatus);		
		}

		if (GetBoxStatus(boxID) == expectStatus)
		{
			InfoL << "一号行车箱子成功打开" << endl;
			break;
		}

		if (i == finalLoop - 1 && GetBoxStatus(boxID) == nowStatus && nowStatus == 0)
		{

			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1001\","
				"\"Result\":\"4\","
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
			return;
		}
		else if (i == finalLoop - 1 && GetBoxStatus(boxID) == nowStatus && nowStatus == 1)
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1001\","
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
			return;
		}
	}
}

bool CKOCDlg::isNormal(int camID)
{

	if (_pCAM[camID]->getlUserID() > 0 && _pCAM[camID]->getlRealPlayHandle() > 0)
	{
		return true;
	}

	for (int i = 0; i < 3; i++)
	{
		if (_pCAM[camID]->getlUserID() < 0)
		{
			_pCAM[camID]->connect();

			switch (camID)
			{
			case 0:
				_pCAM[0]->startvideo(GetDlgItem(IDC_STATIC_CAMLEFT)->m_hWnd);
				break;

			case 1:
				_pCAM[1]->startvideo(GetDlgItem(IDC_STATIC_CAMRIGHT)->m_hWnd);
				break;

			case 2:
				_pCAM[2]->startvideo(GetDlgItem(IDC_STATIC_CAMTAIL)->m_hWnd);
				break;

			default:
				break;
			}
		}
		else if (_pCAM[camID]->getlUserID() > 0 && _pCAM[camID]->getlRealPlayHandle() < 0)
		{
			switch (camID)
			{
			case 0:
				_pCAM[0]->startvideo(GetDlgItem(IDC_STATIC_CAMLEFT)->m_hWnd);
				break;

			case 1:
				_pCAM[1]->startvideo(GetDlgItem(IDC_STATIC_CAMRIGHT)->m_hWnd);
				break;

			case 2:
				_pCAM[2]->startvideo(GetDlgItem(IDC_STATIC_CAMTAIL)->m_hWnd);
				break;

			default:
				break;
			}
		}

		if (_pCAM[camID]->getlUserID() > 0 && _pCAM[camID]->getlRealPlayHandle() > 0)
		{
			return true;
		}

		if (i == 2 && _pCAM[camID]->getlRealPlayHandle() < 0)
		{
			return false;
		}

	}
}

void	CKOCDlg::ParseCommand(CString strcmd)
{

	Document document;
	wstring stringMsg = (LPWSTR)strcmd.GetBuffer(0);

	int size = WideCharToMultiByte(CP_ACP, 0, stringMsg.c_str(), -1, NULL, 0, NULL, NULL);
	char *ch = new char[size + 1];
	if (!WideCharToMultiByte(CP_ACP, 0, stringMsg.c_str(), -1, ch, size, NULL, NULL))
	{
		delete[] ch;
		return;
	}

	string str = ch;
	delete[] ch;


	

	if (document.ParseInsitu((char*)str.c_str()).HasParseError())
	{
		ErrorL << "Unknown message!";
		return;
	}
	if (!document.IsObject())
	{
		ErrorL << "Unknown message!";
		return;
	}
	/*assert(document.HasMember("EventName"));
	assert(document["EventName"].IsString());*/
	string cmd = document["EventName"].GetString();

	//InfoL << "cmd";
	if (cmd.compare("1001") == 0)		//有无渣罐确认请求
	{

		/*DebugL << "CraneID:" << document["CraneID"].GetString();
		DebugL << "SlagID:" << document["SlagID"].GetString();
		DebugL << "TaskState:" << document["TaskState"].GetString();
		DebugL << "AX:" << document["AX"].GetString();
		DebugL << "AY:" << document["AY"].GetString();
		DebugL << "AZ:" << document["AZ"].GetString();
		DebugL << "WZ:" << document["WZ"].GetString();*/

		DebugL << "CR1_ActualZ:" << document["CR1_ActualZ"].GetString();
		float h = atof(document["CR1_ActualZ"].GetString());

		GoToHight_tail(h, 36);

		// 打开尾钩防护柜的箱子
		OperationBox(2, 0, 1, 3);

		Sleep(2000);

		if (isNormal(2))
		{
			OnBnClickedButtonCapimg_tail(0);
			GetImage(0, 1001);
		}
		else
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1001\","
				"\"Result\":\"4\","
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
			return;
		}
		

		// 关闭尾钩的箱子
		OperationBox(2, 1, 0, 3);

		// 返回检测结果
		//_class_id_tail = 2;
		
		if (_class_id_tail > 0 && _class_id_tail == 2)
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1001\","
				"\"Result\":\"0\","				// 1001事件 Result为0表示有罐
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
		}
		else
		{

			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1001\","
				"\"Result\":\"1\","				// 1001事件 Result为1表示无罐
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
		}

	}
	else if (cmd.compare("1004") == 0)		// 耳钩抓钩确认请求
	{
		/*DebugL << "CraneID:" << document["CraneID"].GetString();
		DebugL << "SlagID:" << document["SlagID"].GetString();
		DebugL << "TaskState:" << document["TaskState"].GetString();
		DebugL << "AX:" << document["AX"].GetString();
		DebugL << "AY:" << document["AY"].GetString();
		DebugL << "AZ:" << document["AZ"].GetString();
		DebugL << "WZ:" << document["WZ"].GetString();*/

		DebugL << "CR1_ActualZ:" << document["CR1_ActualZ"].GetString();
		float h = atof(document["CR1_ActualZ"].GetString());

		if (h < 5000)
		{
			GoToHight_left(h, 20);
			GoToHight_right(h, 20);

		}
		else
		{
			GoToHight_left(h, 16);
			GoToHight_right(h, 16);

		}

		// 打开一号柜箱子
		OperationBox(0, 0, 1, 3);
		// 打开二号柜箱子
		OperationBox(1, 0, 1, 3);

		Sleep(2000);
		// 抓图拼接
		if (isNormal(0) && isNormal(1))
		{
			OnBnClickedButtonCapimg();
			_isbusy = true;
			GetLRImage(1004);		//拼接图片之前必须先进行抓图
			_isbusy = false;
		}
		else
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1004\","
				"\"Result\":\"4\","
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
			return;
		}

		// 关闭一号柜箱子
		OperationBox(0, 1, 0, 3);
		// 关闭二号柜箱子
		OperationBox(1, 1, 0, 3);

		// 返回耳钩抓钩结果
		//_class_id_left = 0;
		//_class_id_right = 0;
		
		if (_class_id_left == 0 && _class_id_right == 0)
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1004\","
				"\"Result\":\"0\","					// 1003事件 Result为0 表示耳钩抓钩成功
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
		}
		else if (_class_id_left < 0 && _class_id_right < 0)
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1004\","
				"\"Result\":\"4\","					// 1003事件 Result为4 表示两侧均未检测到目标，中断程序
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
		}
		else
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1004\","
				"\"Result\":\"2\","					// 1003事件 Result为2 表示至少有一侧抓钩失败并尝试重新抓钩
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
		}
	}
	else if (cmd.compare("1003") == 0)			//  耳钩脱钩确认请求
	{
		/*DebugL << "CraneID:" << document["CraneID"].GetString();
		DebugL << "SlagID:" << document["SlagID"].GetString();
		DebugL << "TaskState:" << document["TaskState"].GetString();
		DebugL << "AX:" << document["AX"].GetString();
		DebugL << "AY:" << document["AY"].GetString();
		DebugL << "AZ:" << document["AZ"].GetString();
		DebugL << "WZ:" << document["WZ"].GetString();*/

		DebugL << "CR1_ActualZ:" << document["CR1_ActualZ"].GetString();
		float h = atof(document["CR1_ActualZ"].GetString());

		if (h < 5000)
		{
			GoToHight_left(h, 20);
			GoToHight_right(h, 20);

		}
		else
		{
			GoToHight_left(h, 16);
			GoToHight_right(h, 16);

		}

		// 打开一号柜箱子
		OperationBox(0, 0, 1, 3);
		// 打开二号柜箱子
		OperationBox(1, 0, 1, 3);

		Sleep(2000);
		// 抓图拼接
		if (isNormal(0) && isNormal(1))
		{
			OnBnClickedButtonCapimg();

			_isbusy = true;
			GetLRImage(1003);		//拼接图片之前必须先进行抓图
			_isbusy = false;
		}
		else
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1003\","
				"\"Result\":\"4\","
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
			return;
		}

		// 关闭一号柜箱子
		OperationBox(0, 1, 0, 3);
		// 关闭二号柜箱子
		OperationBox(1, 1, 0, 3);

		//_class_id_left = 1;
		//_class_id_right = 1;
		// 返回耳钩脱钩结果
		if (_class_id_left == 1 && _class_id_right == 1)
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1003\","
				"\"Result\":\"0\","				// 1003事件 Result为0 表示耳钩脱钩成功
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
		}
		else if (_class_id_left < 0 && _class_id_right < 0)
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1003\","
				"\"Result\":\"4\","				// 1003事件 Result为4 表示未检测到目标，中断程序
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
		}
		else
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1003\","
				"\"Result\":\"2\","				// 1003事件 Result为2 表示至少有一侧脱钩失败并尝试重脱钩
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
		}
	}
	else if (cmd.compare("1005") == 0)		// 耳钩能否安全脱钩确认请求
	{
		DebugL << "CR1_ActualZ:" << document["CR1_ActualZ"].GetString();
		float h = atof(document["CR1_ActualZ"].GetString());

		if (h < 5000)
		{
			GoToHight_left(h, 20);
			GoToHight_right(h, 20);

		}
		else
		{
			GoToHight_left(h, 16);
			GoToHight_right(h, 16);

		}

		// 打开一号柜箱子
		OperationBox(0, 0, 1, 3);
		// 打开二号柜箱子
		OperationBox(1, 0, 1, 3);

		Sleep(2000);
		// 抓图拼接
		if (isNormal(0) && isNormal(1))
		{
			OnBnClickedButtonCapimg_nodect();

			_isbusy = true;
			GetLRImage(1005);		//拼接图片之前必须先进行抓图
			_isbusy = false;
		}
		else
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1005\","
				"\"Result\":\"4\","
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
			return;
		}

		// 关闭一号柜箱子
		OperationBox(0, 1, 0, 3);
		// 关闭二号柜箱子
		OperationBox(1, 1, 0, 3);

		string strret = "\{\"CraneID\":\"1\","
			"\"EventName\":\"1005\","
			"\"Result\":\"0\","				// 1003事件 Result为0 表示耳钩脱钩成功
			"\"TZ\":\"5600\"\}";

		kmod::g_msgsender.push(strret);
		
	}
	else if (cmd.compare("1007") == 0)			//1007事件 尾钩抓钩确认请求
	{

		DebugL << "CR1_ActualZ:" << document["CR1_ActualZ"].GetString();
		float h = atof(document["CR1_ActualZ"].GetString());

		if (h > 5000)
		{
			GoToHight_tail(h, 32);
		}
		else
		{
			GoToHight_tail(h, 20);
		}

		// 打开尾钩防护柜的箱子
		OperationBox(2, 0, 1, 3);

		Sleep(2000);
		if (isNormal(2))
		{
			OnBnClickedButtonCapimg_tail(1);
			GetImage(1, 1007);
		}
		else
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1007\","
				"\"Result\":\"4\","
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
			return;
		}
		
		// 关闭尾钩防护柜的箱子
		OperationBox(2, 1, 0, 3);

		// 发送检测结果
		//_class_id_tail = 3;
		
		if (_class_id_tail > 0 && _class_id_tail == 3)
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1007\","
				"\"Result\":\"0\","					// 1007事件 Result为0 表示尾钩抓钩成功
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
		}
		else
		{

			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1007\","
				"\"Result\":\"2\","				  // 1007事件 Result为2 表示尾钩抓钩失败并尝试重新抓钩
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
		}


	}
	else if (cmd.compare("1006") == 0)				// 尾钩脱钩确认请求
	{
		DebugL << "CR1_ActualZ:" << document["CR1_ActualZ"].GetString();
		float h = atof(document["CR1_ActualZ"].GetString());

		if (h > 5000)
		{
			GoToHight_tail(h, 32);
		}
		else
		{
			GoToHight_tail(h, 20);
		}

		// 打开尾钩防护柜的箱子
		OperationBox(2, 0, 1, 3);

		Sleep(2000);
		if (isNormal(2))
		{
			OnBnClickedButtonCapimg_tail(1);
			GetImage(1, 1006);
		}
		else
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1006\","
				"\"Result\":\"4\","
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
			return;
		}
		
		// 关闭尾钩防护柜的箱子
		OperationBox(2, 1, 0, 3);

		//_class_id_tail = 4;
		
		// 发送检测结果
		if (_class_id_tail > 0 && _class_id_tail == 4)
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1006\","
				"\"Result\":\"0\","					// 1006事件 Result为0 表示尾钩脱钩成功
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
		}
		else
		{

			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1006\","
				"\"Result\":\"2\","					// 1006事件 Result为2 表示尾钩脱钩失败并尝试重脱钩
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
		}
	}
	else if (cmd.compare("1008") == 0)		// 尾钩能否安全脱钩确认请求
	{

		DebugL << "CR1_ActualZ:" << document["CR1_ActualZ"].GetString();
		float h = atof(document["CR1_ActualZ"].GetString());

		if (h > 5000)
		{
			GoToHight_tail(h, 32);
		}
		else
		{
			GoToHight_tail(h, 20);
		}

		// 打开尾钩防护柜的箱子
		OperationBox(2, 0, 1, 3);

		Sleep(2000);
		if (isNormal(2))
		{
			OnBnClickedButtonCapimg_tail_nodect();
			GetImage(1, 1008);
		}
		else
		{
			string strret = "\{\"CraneID\":\"1\","
				"\"EventName\":\"1008\","
				"\"Result\":\"4\","
				"\"TZ\":\"5600\"\}";

			kmod::g_msgsender.push(strret);
			return;
		}

		// 关闭尾钩防护柜的箱子
		OperationBox(2, 1, 0, 3);

		string strret = "\{\"CraneID\":\"1\","
			"\"EventName\":\"1006\","
			"\"Result\":\"0\","					// 1006事件 Result为0 表示尾钩脱钩成功
			"\"TZ\":\"5600\"\}";

		kmod::g_msgsender.push(strret);
	}
	else if (cmd.compare("1010") == 0)  //1010事件
	{
		DebugL << "CR1_ActualZ:" << document["CR1_ActualZ"].GetString();
		DebugL << "CR1_VCActualZ:" << document["CR1_VCActualZ"].GetString();

		float h = atof(document["CR1_ActualZ"].GetString());
		float h_tail = atof(document["CR1_VCActualZ"].GetString());
		/*if ((h <= 2100) || (h > 2400 && h < 3400) || (h > 8000 && h < 8900))
		{
			if (h < 5000)
			{
				GoToHight_left(h, 20);
				GoToHight_right(h, 20);
				GoToHight_tail(h, 32);
			}
			else
			{
				GoToHight_left(h, 16);
				GoToHight_right(h, 16);
				GoToHight_tail(h, 32);
			}
			OnBnClickedButtonCapimg_collect();

		}*/

		if (h > 8000 && h < 8900)
		{
			GoToHight_left(h, 16);
			GoToHight_right(h, 16);
			GoToHight_tail(1910, 34);

			OnBnClickedButtonCapimg_collect();
		}

		if ( h < 4000 && h_tail < 4000)
		{
			GoToHight_left(h, 20);
			GoToHight_right(h, 20);
			GoToHight_tail(2085, 34);

			OnBnClickedButtonCapimg_collect();

		}


	}
	InfoL << "事件处理完成" << endl;
}

int CKOCDlg::GetBoxStatus(int id)
{
	int boxStatus;
	if (id < 0 || id > 2) return -1;
	if (_box[id]->alive())
	{
		boxStatus = _box[id]->getStatus();
	}
	return boxStatus;
}

void CKOCDlg::SendBoxCMD(int id, int ncmd)
{

	BufferRaw::Ptr buf;
	if (ncmd == 1)
	{
		//全开
		buf = BufferRaw::Ptr(new BufferRaw("\xFE\x0F\x00\x00\x00\x04\x01\xFF\x31\xD2", 10));
	}
	else {
		//全断
		buf = BufferRaw::Ptr(new BufferRaw("\xFE\x0F\x00\x00\x00\x04\x01\x00\x71\x92", 10));
	}

	int port;
	string ip;
	switch (id)
	{
	case 0:
		ip = KConfig::Instance().cfg()->get<string>("leftbox_ip");
		port = kmod::KConfig::Instance().cfg()->get<int>("leftbox_port");
		break;

	case 1:
		ip = KConfig::Instance().cfg()->get<string>("rightbox_ip");
		port = kmod::KConfig::Instance().cfg()->get<int>("rightbox_port");
		break;

	case 2:
		ip = KConfig::Instance().cfg()->get<string>("tailbox_ip");
		port = kmod::KConfig::Instance().cfg()->get<int>("tailbox_port");
		break;

	default:
		break;
	}

	DebugL << "ip:" << ip << "port:" << port << endl;
	if (!_box[id]->alive())
	{
		_box[id]->startConnect(ip, port);
		Sleep(3000);//异步连接，等连接后发送

	}
	//return;
	if (buf)
	{
		if (_box[id]->alive())
		{
			int ret = _box[id]->send(buf);
			_box[id]->setStatus(ncmd);
			
			//DebugL << "id:" << id << "ret:" << ret << endl;
			if (ret == -1)
			{
				ErrorL << "BOX id:" << id << " send error! MSG:" << hex << buf;
			}
		}
	}


}


void CKOCDlg::OnBnClickedButtonTestlr()
{
	// TODO: 在此添加控件通知处理程序代码

	
}


void CKOCDlg::OnBnClickedBtnTestyolo()
{
	 //TODO: 在此添加控件通知处理程序代码

	/*for (int i = 0; i < 3; i++)
	{
		SendBoxCMD(i, 0);
	}*/


	string folderPath = "D:\\temp";

	CTime CurTime = CTime::GetCurrentTime();
	UpdateData(TRUE);

	char DirName[256];
	sprintf(DirName, "%s\\%04d%02d%02d%02d", folderPath.c_str(), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetHour());

	string dir = DirName;

	if (_access(dir.c_str(), 0) == -1)
	{
		string command;
		command = "mkdir " + dir;
		system(command.c_str());
	}







	//		////////////////测试通讯,处理结果可模拟此信息发送//////////////////
	//		string strret = "\{\"CraneID\":\"1\","
	//			"\"EventName\":\"1001\","
	//			"\"SlagID\":\"8\","
	//			"\"TaskState\":\"1\","
	//			"\"IsCover\":\"1\","
	//			"\"HasSlag\":\"1\","
	//			"\"ErrorID\":\"0\","
	//			"\"ModelStatus\":\"1\","
	//			"\"SlagKind\":\"A\","
	//			"\"TX\":\"14000\","
	//			"\"TY\":\"2800\","
	//			"\"TZ\":\"5600\"\}";
	//		//放入发送队列
	//		kmod::g_msgsender.push(strret);

}

int		CKOCDlg::DetectTest()
{
	
	return 0;
}