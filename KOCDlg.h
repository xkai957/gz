
// KOCDlg.h: 头文件
//

#pragma once

#include "CAMX.h"
#include "KComm.h"
#include "KTCPComm.h"

#include "KTCPServer.h"
#include "KTCPClientBox.h"
#include <Network/TcpServer.h>
#include <Network/TcpSession.h>

#include <string>
#include <darknet/yolo_v2_class.hpp>

#define MAX_CAMSUM (3)
#define UDP_MSG		(WM_USER+1)
#define	TCP_MSG		(WM_USER+2)


// CKOCDlg 对话框
class CKOCDlg : public CDialogEx
{
// 构造
public:
	CKOCDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_KOC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheckLeft();
	afx_msg void OnBnClickedCheckRight();
	afx_msg void OnBnClickedCheckTail();
	afx_msg void OnBnClickedButtonCapvid();
	afx_msg void OnBnClickedButtonGo();
	afx_msg void OnBnClickedButtonCapimg();
	afx_msg void OnBnClickedButtonInfoclear();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonGetpos();
	afx_msg void OnBnClickedButtonStopcapvid();
	afx_msg void OnBnClickedButtonTestcomm();
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();

	void	AddInfoText(CString  infotxt);
	void	ParseCommand(CString strcmd);
	void	SendBoxCMD(int id, int ncmd); //打开或者关闭箱子 ncmd1开，其他关闭 id0-2 表示三个箱子
	int     GetBoxStatus(int id);
	int		GoToHight_left(float h, int zoom);
	int		GoToHight_right(float h, int zoom);
	int		GoToHight_tail(float h, int zoom);
	int		GetLRImage(int event);
	int		GetImage(int id, int event);
	int		DetectTest();
	void	OnBnClickedButtonCapimg_nodect();
	void	OnBnClickedButtonCapimg_tail(int id);
	void	OnBnClickedButtonCapimg_tail_nodect();
	void	OnBnClickedButtonCapimg_collect();
	void	OperationBox(int boxID, int nowStatus, int expectStatus, int finalLoop);
	bool	isNormal(int id);

private:
	CAMX *					_pCAM[3];
	KComm *					_pComm; //
	//KTCPServer *			_server;
	kmod::KTCPClientBox::Ptr		_box[3];	  //BOX的连接
	kmod::KTCPServer::Ptr			_server;
	TcpServer::Ptr					_cmdServer;
	bool							_isbusy;
	bool							_isParsing;
	int 							_class_id_left;
	int								_class_id_right;
	int								_class_id_tail;

	std::string netcfg;
	std::string wgscfg;
public:
	
	float _pospan;
	float _postilt;
	float _poszoom;	

	//TcpServer::Ptr _tcpserver;

	CString m_strInfo;
	BOOL	_camcheck[3];
	CString _caminfo[3];
	std::string SlagPic;
	std::string TailPic;
	std::string LeftPic_dect;
	std::string RightPic_dect;
	std::string	LeftPic_nodect;
	std::string	RightPic_nodect;
	
	afx_msg void OnBnClickedButtonTestlr();
	afx_msg void OnBnClickedBtnTestyolo();
};