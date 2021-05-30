#pragma once
#include <signal.h>
#include <iostream>
#include <Util/logger.h>
#include <Network/TcpClient.h>
using namespace std;
using namespace toolkit;

namespace kmod {
	class KTCPClientBox :public TcpClient
	{
	public:
		typedef std::shared_ptr<KTCPClientBox> Ptr;
		KTCPClientBox() :TcpClient() {
			//DebugL;
		}
		~KTCPClientBox() {
			//DebugL;
		}
		int		getStatus() { return _status; }
		void	setStatus(int status);

	protected:
		virtual void onConnect(const SockException &ex) override;
		virtual void onRecv(const Buffer::Ptr &pBuf) override;
		virtual void onFlush() override;
		virtual void onErr(const SockException &ex) override;
		virtual void onManager() override;
	private:
		int _nTick = 0;
		int	_status = 0;//0---close 1 --- open
	};
}

