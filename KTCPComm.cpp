#include "pch.h"
#include "KTCPComm.h"
#include "KConfig.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm> 
#include <WinSock2.h>
#include "KOCDlg.h"
#include "KOC.h"
#include "Util/TimeTicker.h"
#include "Poller/Timer.h"

#include <Network/Socket.h>


static KTCPComm * g_tcpcomm = new KTCPComm();
KTCPComm * get_defaultTcpCOMM() {
	return g_tcpcomm;
}


static void makeAddr(struct sockaddr *out, const char *ip, uint16_t port) {
	struct sockaddr_in &servaddr = *((struct sockaddr_in *)out);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(ip);
	bzero(&(servaddr.sin_zero), sizeof servaddr.sin_zero);
}
//
//
//
////获取struct sockaddr的IP字符串
//static string getIP(struct sockaddr *addr) {
//	return SockUtil::inet_ntoa(((struct sockaddr_in *)addr)->sin_addr);
//}
//
//static int  getPort(struct sockaddr *addr) {
//	return ntohs(((struct sockaddr_in *)addr)->sin_port);
//
//}
//
//static bool compareaddr(struct sockaddr *addr1, struct sockaddr *addr2)
//{
//	string ip1 = getIP(addr1);
//	string ip2 = getIP(addr2);
//	int port1 = getPort(addr1);
//	int port2 = getPort(addr2);
//
//	if (port1 == port2 && 0 == ip1.compare(ip1))
//	{
//		return true;
//	}
//	else
//		return false;
//}

KTCPComm::KTCPComm()
{

	connvalid = false;
}
KTCPComm::~KTCPComm()
{

}

void  KTCPComm::recvfun(const Buffer::Ptr &buf, struct sockaddr *addr, int len)
{
	//tcp服务发来的数据
	if (buf->size() > 0)
	{


		//CKOCDlg* pdlg =(CKOCDlg*)::AfxGetApp()->GetMainWnd();
		try {
			char* tempbuf = new char[buf->size() + 1];
			memcpy(tempbuf, buf->data(), buf->size());
			tempbuf[buf->size()] = 0;

			CKOCApp* papp = (CKOCApp*)::AfxGetApp();

			if (papp != NULL)
			{
				CString strinfo(tempbuf);
				papp->GetMainWnd()->SendMessage(TCP_MSG, 0, (LPARAM)(LPCTSTR)strinfo);
			}
			delete tempbuf;
		}
		catch (...)
		{
			ErrorL << "recv error!";
		}

	}
}

void KTCPComm::accept(Socket::Ptr &sock)
{
	sock->get_peer_ip();
	sock->get_peer_port();
}


void KTCPComm::init()
{
	string ip = kmod::KConfig::Instance().cfg()->get<string>("tcpclient_ip");
	int port = kmod::KConfig::Instance().cfg()->get<int>("tcpclient_port");

	//makeAddr(&addrServer, ip.c_str(), port);//left 云台激光地址
	InfoL << "tcpclient_ip:" << ip << "tcpclient_port:" << port;
	port = kmod::KConfig::Instance().cfg()->get<int>("mytcpserver_port");
	InfoL << "MYTCP_port:" << port;

	sockRecv = Socket::createSocket();//创建一个UDP数据接收端口
	sockRecv->listen(port);

	//sockSend = Socket::createSocket();//创建一个UDP数据发送端口
	//sockSend->bindUdpSock(0);//本地接收和发送的UDP绑定

	//sockSend->setSendPeerAddr(&addrDst);
	sockRecv->setOnAccept([](Socket::Ptr &sock) {
		g_tcpcomm->accept(sock);
	});
	sockRecv->setOnRead([](const Buffer::Ptr &buf, struct sockaddr *addr, int len) {
		//接收到数据回调		
		g_tcpcomm->recvfun(buf, addr, len);
		//InfoL << " s:" << buf->size();
	});

	//sockRecv->setOnErr([](const SockException &err) {
	//	//ErrorL << err.what();
	//	g_udpcomm->onerr();
	//	//int ret = sockRecv->send("ww", &addrServer, sizeof(struct sockaddr_in));
	//});

	//sockSend->setSendPeerAddr(&addrServer, sizeof(struct sockaddr_in));
	//发送数据到服务器 
	//sockSend->send("ww", &addrServer, sizeof(struct sockaddr_in));
}

void KTCPComm::onerr()
{


}

void KTCPComm::sendCMD(string buf)
{
	Socket::Ptr socks = Socket::createSocket();//创建一个UDP数据接收端口
	socks->bindUdpSock(0);//本地接收和发送的UDP绑定
	socks->send(buf.c_str(), &addrServer, sizeof(struct sockaddr_in));
}

