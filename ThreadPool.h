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


#include "Queue.h"


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
			std::shared_ptr<std::thread> t = std::make_shared<std::thread>(std::bind(&ThreadPool::Run, this, i));
			ThreadVec_.emplace_back(t);
		}
	}
	void Terminate()
	{
		std::call_once(CallFlag_, [this] { this->TerminateAll(); });
	}

	void AddTask(const std::function<void()>& _task)
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
				std::unique_lock<std::mutex> locker(ThreadMutex_);
				if (ThreadCondition_.wait_for(locker, std::chrono::seconds(60)) == std::cv_status::timeout)
					continue;
			}

			if (IsStop_)
				break;

			std::function<void()> task(nullptr);
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
	std::atomic<bool> IsStop_;
	std::atomic<bool> IsTerminated_;

	size_t ThreadCount_;

	std::vector<std::shared_ptr<std::thread>>	ThreadVec_;
	std::mutex								ThreadMutex_;
	std::condition_variable					ThreadCondition_;

	Queue<std::function<void()>> TaskQueue_;

	std::once_flag   CallFlag_;
};

