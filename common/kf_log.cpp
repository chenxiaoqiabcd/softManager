#include "kf_log.h"

#include <iostream>
#include <Shlwapi.h>

#include "kf_str.h"
#include "stringHelper.h"

#define MAX_LEN MAX_PATH

void KfLog::Output(const char* type, const char* time, const char* file, const char* function, int line) {
 // 	auto err = GetLastError();
	// if(0 == err) {
	// 	std::ignore = printf_s("[%s %s]%s:%d %s ", type, time, PathFindFileNameA(file), line, function);
	// 	return;
	// }
	//
	// std::ignore = printf_s("[%s %s]%s:%d %s last err: %d ",
	// 					   type, time, PathFindFileNameA(file), line, function, err);

	std::ignore = printf_s("[%s %s]%s:%d %s ", type, time, PathFindFileNameA(file), line, function);
}

void KfLog::Output(char const* const format, ...) {
	va_list list;
	va_start(list, format);
	const auto result = KfString::FormatList(format, list);
	va_end(list);

	std::cout << result << "\n";
}

void KfLog::Output(const wchar_t* format, ...) {
	va_list list;
	va_start(list, format);
	const auto result = KfString::FormatList(format, list);
	va_end(list);

	std::cout << CStringHelper::w2a(result).c_str() << "\n";
}

void KfLog::SetInfoTextAttribute() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
}

void KfLog::SetWarnTextAttribute() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN);
}

void KfLog::SetErrorTextAttribute() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
}

void KfLog::ResetState() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
							FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

KfTimer::KfTimer(const char* index) {
	index_ = index;
	KF_INFO("start executable: %s", index_.c_str());
	start_time_ = GetTickCount();
}

KfTimer::KfTimer(const char* format, ...) {
	std::unique_ptr<char> value(new char[MAX_LEN]);
	memset(value.get(), 0, MAX_LEN);

	va_list list;
	va_start(list, format);
	std::ignore = vsnprintf(value.get(), MAX_LEN, format, list);
	va_end(list);

	index_ = value.get();
	KF_INFO("start executable: %s", index_.c_str());
	start_time_ = GetTickCount();
}

KfTimer::KfTimer(const wchar_t* format, ...) {
	std::unique_ptr<wchar_t> value(new wchar_t[MAX_LEN]);
	memset(value.get(), 0, MAX_LEN);

	va_list list;
	va_start(list, format);
	std::ignore = vswprintf(value.get(), MAX_LEN, format, list);
	va_end(list);

	index_ = KfString(value.get()).GetString();
	KF_INFO("start executable: %s", index_.c_str());
	start_time_ = GetTickCount();
}

KfTimer::~KfTimer() {
	auto diff_time = GetTickCount() - start_time_;
	if(diff_time > time_out_) {
		KF_WARN("finished executable: %s, time out: %dms", index_.c_str(), diff_time);
		return;
	}

	KF_INFO("finished executable: %s, time: %dms", index_.c_str(), diff_time);
}


void logTest() {
	KF_TIMER(L"%s", __FUNCTIONW__);

	KF_INFO("hello %s", "world");
	KF_WARN("hello %s", "world");
	KF_ERROR("hello %s", "world");
}
