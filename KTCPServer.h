#pragma once
#include <signal.h>
#include <iostream>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <Util/logger.h>
#include <Util/TimeTicker.h>
#include <Network/TcpServer.h>
#include <Network/TcpSession.h>
#include  "KMsg.h"

using namespace std;
using namespace toolkit;

namespace kmod {

	class KTCPServer : public TcpSession 
	{
		public:
			typedef std::shared_ptr<KTCPServer> Ptr;
			KTCPServer(const Socket::Ptr &sock) :TcpSession(sock) {
				//定期检查发送队列，若有数据则发送
				_timer0 = std::make_shared<Timer>(0.1, [&]()
				{

					//从发送队列取出数据发送
					std::shared_ptr<string> str = kmod::g_msgsender.pop();
					if (str == nullptr) { return true; }

					BufferRaw::Ptr buff = BufferRaw::Ptr(new BufferRaw(str->c_str(), str->size()));
					InfoL << "开始发送处理结果信息" << endl;
					/*if (send(buff) == str->size())
					{
						InfoL << 11111 << endl;
						InfoL << buff->data();
						DebugL << " Send message successfully!";
					}*/


					int sendSize = send(buff);
					InfoL << "第一次尝试发送处理结果消息 sendSize = " << sendSize << endl;
					if (sendSize > 0)
					{
						InfoL << "sendSize = " << sendSize << endl;
						InfoL << buff->data();
						DebugL << " Send message successfully!";
					}
					else
					{
						for (int i = 0; i < 10; i++)
						{
							Sleep(500);
							if (sendSize < 0)
							{
								sendSize = send(buff);
								InfoL << "第" << i + 2 << "次尝试发送处理结果消息 sendSize = " << sendSize << endl;
							}
							else
							{
								break;
							}

						}
					}

					return true;
				}, nullptr);
			}
			~KTCPServer() {}
			virtual void onRecv(const Buffer::Ptr &buf) override;
			virtual void onError(const SockException &err) override;
			virtual void onManager() override;
			void init();

		private:
			Ticker		_ticker;
			Timer::Ptr  _timer0;
			TcpServer::Ptr _tcpserver;
	};

	//KTCPServer * get_defaultTCPCOMM();
};