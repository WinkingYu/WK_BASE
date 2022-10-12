#pragma once

#include <queue>
#include <mutex>
#include <deque>
#include <string>
#include <iomanip>
#include <numeric>
#include <algorithm>
#include <iostream>

class Pipeline
{
public:
	Pipeline(Pipeline const&) = delete;
	Pipeline(Pipeline&&) = delete;
	Pipeline& operator=(Pipeline const&) = delete;
	Pipeline& operator=(Pipeline&&) = delete;

	Pipeline()
	{}

	virtual ~Pipeline()
	{
		deque_.clear();
	}

	size_t Push(uint8_t _in)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		deque_.push_back(_in);

		return deque_.size();
	}

	template <typename T>
	size_t Push(const T& _in)
	{
		uint8_t* add = (uint8_t*)(&_in);

		std::lock_guard<std::mutex> lock(mutex_);
		deque_.resize(deque_.size() + sizeof(T));
		std::copy_backward(add, add + sizeof(T), deque_.end());

		return sizeof(T);
	}

	size_t Push(const char* _buf, int _len)
	{
		std::lock_guard<std::mutex> lock(mutex_);

		deque_.resize(deque_.size() + _len);
		std::copy_backward(_buf, _buf + _len, deque_.end());

		return static_cast<size_t>(_len);
	}

	template <typename T>
	size_t Push(const std::vector<T>& _vector)
	{
		size_t pushCount{ 0 };

		std::lock_guard<std::mutex> lock(mutex_);
		for (auto it : _vector)
		{
			uint8_t* add = (uint8_t*)(&it);

			deque_.resize(deque_.size() + sizeof(T));
			std::copy_backward(add, add + sizeof(T), deque_.end());

			pushCount += sizeof(T);
		}

		return pushCount;
	}

	size_t Push(const std::string& _str)
	{
		std::lock_guard<std::mutex> lock(mutex_);

		size_t strLen = _str.length();

		deque_.resize(deque_.size() + strLen);
		std::copy_backward(_str.begin(), _str.end(), deque_.end());

		return strLen;
	}

	size_t Push(const std::vector<std::string>& _vector)
	{
		size_t pushCount{ 0 };
		for (auto it : _vector)
			pushCount += Push(it);

		return pushCount;
	}

	template <typename T>
	const std::deque<uint8_t>::iterator Search(const T& _in)
	{
		uint8_t* add = (uint8_t*)(&_in);

		std::deque<uint8_t> temp;
		temp.resize(sizeof(T));
		std::copy(add, add + sizeof(T), temp.end());

		return std::search(deque_.begin(), deque_.end(), temp.begin(), temp.end());
	}

	const std::deque<uint8_t>::iterator Search(const char* _buf, const int _len)
	{
		std::deque<uint8_t> temp;
		temp.resize(_len);
		std::copy(_buf, _buf + _len, temp.end());

		return std::search(deque_.begin(), deque_.end(), temp.begin(), temp.end());
	}

	const std::deque<uint8_t>::iterator Search(const std::vector<std::string>& _vector)
	{
		std::deque<uint8_t> temp{};

		for (auto it : _vector)
		{
			temp.resize(temp.size() + it.length());
			std::copy(it.begin(), it.end(), temp.end());
		}

		return std::search(deque_.begin(), deque_.end(), temp.begin(), temp.end());
	}

	template <typename T>
	const std::deque<uint8_t>::iterator Search(const std::vector<T>& _vector)
	{
		std::deque<uint8_t> temp;

		for (auto it : _vector)
		{
			uint8_t* add = (uint8_t*)(&it);

			temp.resize(temp.size() + sizeof(T));
			std::copy(add, add + sizeof(T), temp.end());
		}

		return std::search(deque_.begin(), deque_.end(), temp.begin(), temp.end());
	}

	const std::deque<uint8_t>::iterator Search(const std::string& _str)
	{
		std::deque<uint8_t> temp;
		temp.resize(_str.length());
		std::copy(_str.begin(), _str.end(), temp.end());

		return std::search(deque_.begin(), deque_.end(), temp.begin(), temp.end());
	}

	long long At(std::deque<uint8_t>::iterator _it)
	{
		if (_it != deque_.end())
			return std::distance(deque_.begin(), _it);

		return -1;
	}

	size_t Pop(char* _buf, int _len)
	{
		std::lock_guard<std::mutex> lock(mutex_);

		size_t dSize(deque_.size());

		if (_len >= static_cast<int>(dSize))
		{
			std::copy(deque_.begin(), deque_.end(), _buf);
			deque_.erase(deque_.begin(), deque_.end());
		}
		else
		{
			std::copy(deque_.begin(), deque_.begin() + _len, _buf);
			deque_.erase(deque_.begin(), deque_.begin() + _len);

			dSize = _len;
		}

		return dSize;
	}

	size_t IsEmpty() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return deque_.empty();
	}

	void Clear()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		deque_.clear();
	}

	size_t Length() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return deque_.size();
	}

	uint64_t SumAll()
	{
		return accumulate(deque_.begin(), deque_.end(), 0);
	}

	friend std::ostream& operator<<(std::ostream& _os, Pipeline& _pipeline)
	{
		std::ios_base::fmtflags f{ _os.flags() };

		for (auto i : _pipeline.deque_)
		{
			_os << std::right << std::setw(2) << std::setfill('0') << std::hex << +(i) << ' ';
		}

		_os.flags(f);
		//_os << endl;

		return _os;
	}

private:
	std::deque<uint8_t> deque_;
	mutable std::mutex mutex_;
};
