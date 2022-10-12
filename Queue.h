#pragma once

#include <queue>
#include <mutex>
#include <deque>

template <class T>
class Queue
{
public:
	Queue(Queue const&) = delete;
	Queue(Queue&&) = delete;
	Queue& operator=(Queue const&) = delete;
	Queue& operator=(Queue&&) = delete;

	Queue()
		: queue_(), mutex_()
	{}

	virtual ~Queue()
	{
		while (!queue_.empty())
			queue_.pop();
	}

	void Push(T _record)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.emplace(std::move(_record));
	}

	bool Pop(T& _record)
	{
		std::lock_guard<std::mutex> lock(mutex_);

		if (queue_.empty())
			return false;

		_record = std::move(queue_.front());
		queue_.pop();

		return true;
	}

	size_t Size()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.size();
	}

	bool IsEmpty()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.empty();
	}

	void Clear()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		while (!queue_.empty())
			queue_.pop();
	}

private:
	std::queue<T> queue_;
	std::mutex mutex_;
};

