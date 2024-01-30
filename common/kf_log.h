#pragma once

#include <cstdint>
#include <string>

class __declspec (dllexport) KfLog {
public:
	static void Output(const char* type, const char* time, const char* file, const char* function, int line);

	static void Output(uint32_t len, char const* const format, ...);

	static void Output(uint32_t len, const wchar_t* format, ...);

	static void SetInfoTextAttribute();

	static void SetWarnTextAttribute();

	static void SetErrorTextAttribute();

	static void ResetState();
};

class KfTimer {
public:
	KfTimer(const char* index);

	KfTimer(const char* format, ...);

	KfTimer(const wchar_t* format, ...);

	~KfTimer();
private:
	int time_out_ = 500;
	unsigned long start_time_;
	std::string index_;
};

#define KF_INFO(...) KfLog::SetInfoTextAttribute(),\
	KfLog::Output("info", __TIME__, __FILE__, __FUNCSIG__, __LINE__), \
	KfLog::Output(260, __VA_ARGS__), \
	KfLog::ResetState() \
	//

#define KF_INFO_L(len, ...) KfLog::SetInfoTextAttribute(),\
	KfLog::Output("info", __TIME__, __FILE__, __FUNCSIG__, __LINE__), \
	KfLog::Output(len, __VA_ARGS__), \
	KfLog::ResetState() \
	//

#ifdef _DEBUG
	#define KF_DEBUG(...) KfLog::SetInfoTextAttribute(),\
		KfLog::Output("debug", __TIME__, __FILE__, __FUNCSIG__, __LINE__), \
		KfLog::Output(__VA_ARGS__), \
		KfLog::ResetState() \
		//
#else
	#define KF_DEBUG(...) while(false)
#endif

#define KF_WARN(...) KfLog::SetWarnTextAttribute(),\
	KfLog::Output("warning", __TIME__, __FILE__, __FUNCSIG__, __LINE__),\
	KfLog::Output(260, __VA_ARGS__), \
	KfLog::ResetState() \
	//

#define KF_WARN_L(len, ...) KfLog::SetWarnTextAttribute(),\
	KfLog::Output("warning", __TIME__, __FILE__, __FUNCSIG__, __LINE__),\
	KfLog::Output(len, __VA_ARGS__), \
	KfLog::ResetState() \
	//

#define KF_ERROR(...) KfLog::SetErrorTextAttribute(),\
	KfLog::Output("error", __TIME__, __FILE__, __FUNCSIG__, __LINE__), \
	KfLog::Output(260, __VA_ARGS__), \
	KfLog::ResetState() \
	//

#define KF_ERROR_L(len, ...) KfLog::SetErrorTextAttribute(),\
	KfLog::Output("error", __TIME__, __FILE__, __FUNCSIG__, __LINE__), \
	KfLog::Output(len, __VA_ARGS__), \
	KfLog::ResetState() \
	//

#define KF_TIMER(...) KfTimer kf_timer(__VA_ARGS__)

void __declspec (dllexport) logTest();