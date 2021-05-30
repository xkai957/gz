#pragma once
#include <signal.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <Util/util.h>
#include <Util/logger.h>
#include <Network/Socket.h>

#include <Util/TimeTicker.h>
#include <Poller/Timer.h>

using namespace std;
using namespace toolkit;


class KTCPComm
{
public:
	KTCPComm();
	~KTCPComm();
	//���պ���
	void recvfun(const Buffer::Ptr &buf, struct sockaddr *addr, int);
	void init();
	void sendCMD(string);
	void accept(Socket::Ptr &sock);

	void onerr();

private:
	Socket::Ptr sockRecv;		//���ذ󶨵�socket
	Socket::Ptr sockSend;		//���ذ󶨵�socket
	//Socket::Ptr sock;

	struct sockaddr addrServer;//�����̨

	bool	connvalid;


};

KTCPComm * get_defaultTcpCOMM();

