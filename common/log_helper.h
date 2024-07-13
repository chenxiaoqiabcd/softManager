#pragma once

#include <cstdint>
#include <string>

class KfLog {
public:
	static void EnableLocalLog();

	static void ClearLocalLog();

	static void Output(const char* type, const char* file, const char* function, int line);

	static void Output(char const* const format, ...);

	static void Output(const wchar_t* format, ...);

	static void Output(const std::string& value);

	static void Output(const std::wstring& value);

	static void SetInfoTextAttribute();

	static void SetWarnTextAttribute();

	static void SetErrorTextAttribute();

	static void ResetState();

	static bool HasErrorLog();

	static std::string GetLogPath();
private:
	static bool AppendData(const char* path, const char* data);

	inline static std::string out_file_name_;
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
	KfLog::Output("info", __FILE__, __FUNCTION__, __LINE__), \
	KfLog::Output(__VA_ARGS__), \
	KfLog::ResetState() \
	//

#ifdef _DEBUG
#define KF_DEBUG(...) KfLog::SetInfoTextAttribute(),\
		KfLog::Output("debug", __FILE__, __FUNCTION__, __LINE__), \
		KfLog::Output(__VA_ARGS__), \
		KfLog::ResetState() \
		//
#else
#define KF_DEBUG(...) while(false)
#endif

#define KF_WARN(...) KfLog::SetWarnTextAttribute(),\
	KfLog::Output("warning", __FILE__, __FUNCTION__, __LINE__),\
	KfLog::Output(__VA_ARGS__), \
	KfLog::ResetState() \
	//

#define KF_ERROR(...) KfLog::SetErrorTextAttribute(),\
	KfLog::Output("error", __FILE__, __FUNCTION__, __LINE__), \
	KfLog::Output(__VA_ARGS__), \
	KfLog::ResetState() \
	//

#define KF_TIMER(...) KfTimer kf_timer(__VA_ARGS__)

void __declspec (dllexport) logTest();