#include "pch.h"

#include "KTCPClientBox.h"
namespace kmod {



	void KTCPClientBox::onConnect(const SockException &ex)  {
		//���ӽ���¼�
		InfoL << (ex ? ex.what() : "success");
	}
	void KTCPClientBox::onRecv(const Buffer::Ptr &pBuf)  {
		//���������¼�,����box���͹���������,����״ֵ̬
		DebugL << pBuf->data() << " from port:" << get_peer_port();

	}
	void KTCPClientBox::onFlush()  {
		//���������󣬻�������¼�
		//DebugL;
	}
	void KTCPClientBox::onErr(const SockException &ex)  {
		//�Ͽ������¼�
		WarnL << "BOX�Ͽ�";
	}

	void KTCPClientBox::setStatus(int status)
	{
		_status = status;
	}
	void KTCPClientBox::onManager()  {
		////��ʱ�������ݵ�box ����ѯ״̬
		BufferRaw::Ptr buf =  BufferRaw::Ptr(new BufferRaw("\xFE\x02\x00\x00\x00\x04\x6D\xC6", 10));

		if (buf){
			if ((this)->send(buf) == 10)
			{
				// DebugL << "send OK!";
			}
		}
	}

};
