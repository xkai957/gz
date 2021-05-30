#include "pch.h"

#include "KTCPServer.h"
#include "KOCDlg.h"
#include "KOC.h"
#include "KConfig.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm> 
#include <WinSock2.h>

#include <Util/TimeTicker.h>
#include <Poller/Timer.h>

#include <Network/TcpServer.h>
#include <Network/TcpSession.h>
#include <Network/Socket.h>

#include <rapidjson/document.h>     
#include <rapidjson/prettywriter.h> 
#include <rapidjson/rapidjson.h>

using namespace rapidjson;

namespace kmod {

	/*static KTCPServer * g_TCPcomm = 
	KTCPServer * get_defaultTCPCOMM() {
		return g_TCPcomm;
	}*/
	
	static semaphore sem;
	void KTCPServer::onRecv(const Buffer::Ptr &buf) {
		
		if (buf->size() > 0)
		{
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

	

	void  KTCPServer::init()
	{
		// ��ʼ����־ģ��
		Logger::Instance().add(std::make_shared<ConsoleChannel>());
		Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

		int myserverport = kmod::KConfig::Instance().cfg()->get<int>("mytcpserver_port");
		InfoL << "�����������˿�:" << myserverport << endl;
		TcpServer::Ptr server(new TcpServer());
		_tcpserver = TcpServer::Ptr(server);
		_tcpserver->start<kmod::KTCPServer>(myserverport);//����9002�˿�
		InfoL << "��ʼ����9002�˿�" << endl;
		//�˳������¼�����
		
		signal(SIGINT, [](int) { sem.post(); });// �����˳��ź�
		
		
	}
	void  KTCPServer::onError(const SockException &err) 
	{
		WarnL <<"JSON�ͻ��˶Ͽ�����"; //�ͻ��˶Ͽ�����
	}
	void  KTCPServer::onManager() 
	{
		//��ʱ���		
	}
}
