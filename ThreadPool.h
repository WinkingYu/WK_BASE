#pragma once

#include <future>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <type_traits>

#include "STL.h"

class ThreadPool
{
public:
	ThreadPool(ThreadPool const&) = delete;
	ThreadPool(ThreadPool&&) = delete;
	ThreadPool& operator=(ThreadPool const&) = delete;
	ThreadPool& operator=(ThreadPool&&) = delete;

public:
	ThreadPool()
		:IsStop_(false), IsTerminated_(false), ThreadCount_(1u)
	{}
	virtual ~ThreadPool()
	{}

	void Start(size_t _num)
	{
		if (_num < 1)
			ThreadCount_ = 1;
		else if (_num > 30)
			ThreadCount_ = 30;
		else
			ThreadCount_ = _num;

		for (size_t i = 0; i < ThreadCount_; ++i)
		{
			shared_ptr<thread> t = make_shared<thread>(bind(&ThreadPool::Run, this, i));
			ThreadVec_.emplace_back(t);
		}
	}
	void Terminate()
	{
		call_once(CallFlag_, [this] { this->TerminateAll(); });
	}

	void AddTask(const function<void()>& _task)
	{
		if (!IsStop_)
		{
			TaskQueue_.Push(_task);
			ThreadCondition_.notify_one();
		}
	}

	size_t TaskConut()
	{
		return TaskQueue_.Size();
	}

private:
	void Run(int _id)
	{
		while (true)
		{
			if (IsStop_)
				break;

			if (TaskQueue_.IsEmpty())
			{
				unique_lock<mutex> locker(ThreadMutex_);
				if (ThreadCondition_.wait_for(locker, chrono::seconds(60)) == cv_status::timeout)
					continue;
			}

			if (IsStop_)
				break;

			function<void()> task(nullptr);
			if (TaskQueue_.Pop(task) && task != nullptr)
				task();
		}
	}

	void TerminateAll()
	{
		IsStop_ = true;

		ThreadCondition_.notify_all();

		for (auto& iter : ThreadVec_)
		{
			if (iter != NULL && iter->joinable())
				iter->join();
		}

		ThreadVec_.clear();

		IsTerminated_ = true;
	}

private:
	atomic<bool> IsStop_;
	atomic<bool> IsTerminated_;

	size_t ThreadCount_;

	std::vector<shared_ptr<std::thread>>	ThreadVec_;
	std::mutex								ThreadMutex_;
	std::condition_variable					ThreadCondition_;

	Queue<function<void()>> TaskQueue_;

	once_flag   CallFlag_;
};

