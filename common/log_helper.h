#pragma once

#include <mutex>
#include <string>

class KfLog {
public:
	static void EnableLocalLog();

	static void ClearLocalLog();

	static void Output(const char* type, const char* file, const char* function, int line, const char* format, ...);

	static void Output(const char* type, const char* file, const char* function, int line, const wchar_t* format, ...);

	static void Output(const char* type, const char* file, const char* function, int line, const std::string& value);

	static void Output(const char* type, const char* file, const char* function, int line, const std::wstring& value);

	static void SetInfoTextAttribute();

	static void SetWarnTextAttribute();

	static void SetErrorTextAttribute();

	static void ResetState();

	static bool HasErrorLog();

	static std::string GetLogPath();
private:
	static bool AppendData(const char* path, const char* data);

	static std::wstring ContactLogHeader(const char* type, const char* file, const char* function, int line);

	inline static std::string out_file_name_;

	inline static std::mutex mtx_;
};

class KfTimer {
public:
	KfTimer(const std::string& value);

	KfTimer(const std::wstring& value);

	KfTimer(const char* format, ...);

	KfTimer(const wchar_t* format, ...);

	~KfTimer();
private:
	unsigned long time_out_ = 500;
	unsigned long start_time_;
	std::string index_;
};

#define KF_INFO(...) KfLog::SetInfoTextAttribute(),\
	KfLog::Output("info", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__), \
	KfLog::ResetState() \
	//

#define KF_WARN(...) KfLog::SetWarnTextAttribute(),\
	KfLog::Output("warning", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__),\
	KfLog::ResetState() \
	//

#define KF_ERROR(...) KfLog::SetErrorTextAttribute(),\
	KfLog::Output("error", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__), \
	KfLog::ResetState() \
	//

#define KF_TIMER(...) KfTimer kf_timer(__VA_ARGS__)