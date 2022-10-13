#pragma once

#include <iostream>
#include <string>
#include <future>
#include <chrono>
#include <condition_variable>
#include <cstdarg>
#include <functional>
#include <iomanip>

#include "Queue.h"

using namespace std;

class LogData
{
protected:
	time_t   time_;
	string   msg_;


public:
	LogData(LogData const&) = delete;
	LogData(LogData&&) = delete;
	LogData& operator=(LogData const&) = delete;
	LogData& operator=(LogData&&) = delete;

	LogData() = delete;

	LogData(const string& _log)
		: time_(time(0))
		, msg_(_log)
	{}

	virtual ~LogData()
	{}

	virtual void Output() = 0;
};

class InfLog final :public LogData
{
public:
	InfLog(const string& _log)
		: LogData(_log)
	{}

	void Output() override
	{
		tm t;
		localtime_r(&time_, &t);

		cout << put_time(&t, "[%F %X]")
			<< "\tINF\t"
			<< msg_
			<< '\n';
	}
};

class ErrLog final :public LogData
{
public:
	ErrLog(const string& _log)
		:LogData(_log)
	{}

	void Output() override
	{
		tm t;
		localtime_r(&time_, &t);

		cout << put_time(&t, "[%F %X]")
			<< "\tERR\t"
			<< msg_
			<< '\n';
	}
};

class LogManager
{
public:
	using PLogData = LogData*;

	static constexpr int LOG_BUF_SIZE = 1024 * 2000;

public:

	LogManager& operator=(const LogManager&) = delete;
	LogManager(const LogManager&) = delete;
	LogManager& operator=(LogManager const&) = delete;
	LogManager& operator=(LogManager&&) = delete;
	

	static LogManager* Instance()
	{
		static LogManager g_LogManager;
		return &g_LogManager;
	}

	void LogInf(const char* _format, ...)
	{
		char* tem = new char[LOG_BUF_SIZE];
		va_list pvar;
		va_start(pvar, _format);
		vsnprintf(tem, LOG_BUF_SIZE, _format, pvar);
		va_end(pvar);

		string strLog(tem);

		delete[] tem;
		tem = nullptr;

		PLogData newLog(new InfLog(strLog));
		//newLog->Output();
		queue_.Push(newLog);
		condition_.notify_one();
	}

	void LogErr(const char* _format, ...)
	{
		char* tem = new char[LOG_BUF_SIZE];
		va_list pvar;
		va_start(pvar, _format);
		vsnprintf(tem, LOG_BUF_SIZE, _format, pvar);
		va_end(pvar);

		string strLog(tem);

		delete[] tem;
		tem = nullptr;

		PLogData newLog(new ErrLog(strLog));
		//newLog->Output();
		queue_.Push(newLog);
		condition_.notify_one();
	}

private:
	LogManager()
		: shutdown_(false)
		, queue_()
		, dispalyThread_(async(launch::async | launch::deferred, bind(&LogManager::DisplayThreadFunc, this)))
	{}

	virtual ~LogManager()
	{
		shutdown_ = true;
		condition_.notify_all();
	}

	void DisplayThreadFunc()
	{
		while (true)
		{
			unique_lock<mutex> lock(mutex_);

			if (queue_.IsEmpty())
				condition_.wait(lock);

			if (shutdown_)
				break;

			while (!queue_.IsEmpty())
			{
				PLogData pLog{ nullptr };
				if (queue_.Pop(pLog) && pLog != nullptr)
					pLog->Output();
				delete pLog;
				pLog = nullptr;
			}
		}
	}

private:
	atomic<bool>        shutdown_;
	Queue<PLogData>     queue_;

	mutex               mutex_;
	condition_variable  condition_;
	future<void>        dispalyThread_;

};


#define LOGI(tmplt, ...) \
LogManager::Instance()->LogInf(tmplt, ##__VA_ARGS__);

#define LOGE(tmplt, ...) \
LogManager::Instance()->LogErr(tmplt, ##__VA_ARGS__);
