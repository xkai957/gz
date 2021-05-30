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


class KComm
{
public:
	KComm();
	~KComm();
	//���պ���
	void recvfun(const Buffer::Ptr &buf, struct sockaddr *addr, int);
	void init();
	void sendCMD(string);

	void onerr();

private:
	Socket::Ptr sockRecv;		//���ذ󶨵�socket
	Socket::Ptr sockSend;		//���ذ󶨵�socket

	struct sockaddr addrServer;//�����̨

	bool	connvalid;


};

KComm * get_defaultCOMM();
