#include "pch.h"

#include "KTCPClientBox.h"
namespace kmod {



	void KTCPClientBox::onConnect(const SockException &ex)  {
		//连接结果事件
		InfoL << (ex ? ex.what() : "success");
	}
	void KTCPClientBox::onRecv(const Buffer::Ptr &pBuf)  {
		//接收数据事件,解析box发送过来的数据,更新状态值
		DebugL << pBuf->data() << " from port:" << get_peer_port();

	}
	void KTCPClientBox::onFlush()  {
		//发送阻塞后，缓存清空事件
		//DebugL;
	}
	void KTCPClientBox::onErr(const SockException &ex)  {
		//断开连接事件
		WarnL << "BOX断开";
	}

	void KTCPClientBox::setStatus(int status)
	{
		_status = status;
	}
	void KTCPClientBox::onManager()  {
		////定时发送数据到box ，查询状态
		BufferRaw::Ptr buf =  BufferRaw::Ptr(new BufferRaw("\xFE\x02\x00\x00\x00\x04\x6D\xC6", 10));

		if (buf){
			if ((this)->send(buf) == 10)
			{
				// DebugL << "send OK!";
			}
		}
	}

};
