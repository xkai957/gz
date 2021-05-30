#pragma once

#include <list>
#include <mutex>
#include <string>
#include <algorithm>
#include <Util/logger.h>


using namespace std;
using namespace toolkit;
namespace kmod {

	template <typename T>
	class KMsg
	{
		
	public:
		const int MAX_QUEQUE = 3;
		KMsg() {}
		~KMsg() {}
		bool push(T& value)
		{
			std::unique_lock<std::mutex> lock(mutex);

			TraceL << "prepush size:" << data.size();

			if (data.size() < MAX_QUEQUE)
			{
				data.push_back(value);
				lock.unlock();
				return true;
			}
			else
			{
				ErrorL << "接收队列已满";
				lock.unlock();
				return false;
			}
			
			
		}
		std::shared_ptr<T> pop()
		{
			std::unique_lock<std::mutex> lock(mutex);
			if (data.empty()) {
				lock.unlock();
				return nullptr;
			}
			std::shared_ptr<T> const res(std::make_shared<T>(data.front()));
			
			TraceL << "prepop size:" << data.size();// << " " << data.front();
			data.pop_front();
			lock.unlock();
			return res;
		}
	private:
		std::list<T> data;
		std::mutex mutex;			
	};

	extern KMsg<std::string>	g_msgrecver;
	extern KMsg<std::string>	g_msgsender;

}

