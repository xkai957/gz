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
	//接收函数
	void recvfun(const Buffer::Ptr &buf, struct sockaddr *addr, int);
	void init();
	void sendCMD(string);

	void onerr();

private:
	Socket::Ptr sockRecv;		//本地绑定的socket
	Socket::Ptr sockSend;		//本地绑定的socket

	struct sockaddr addrServer;//左边云台

	bool	connvalid;


};

KComm * get_defaultCOMM();
