#include "log_helper.h"

#include <chrono>
#include <filesystem>
#include <iostream>

#include <Shlwapi.h>

#include "file_read_stream.h"
#include "helper.h"
#include "kf_str.h"
#include "StringHelper.h"

#define MAX_LEN MAX_PATH

void KfLog::EnableLocalLog() {
	auto folder = std::filesystem::path(Helper::GetRoamingDir()) / "softManager";

	if (!std::filesystem::is_directory(folder)) {
		std::error_code error_code;
		std::filesystem::create_directories(folder, error_code);
	}

	out_file_name_ = (folder / "log.data").string();

	std::filesystem::remove(out_file_name_);
}

void KfLog::ClearLocalLog() {
	DeleteFileA(out_file_name_.c_str());
}

void KfLog::Output(const char* type, const char* file, const char* function, int line, const char* format, ...) {
	KfString log_header = ContactLogHeader(type, file, function, line).c_str();

	va_list list;
	va_start(list, format);

	auto log_content = KfString::FormatList(format, list);

	va_end(list);

	auto result = log_header + L" " + log_content.c_str() + L"\n";

	AppendData(out_file_name_.c_str(), result);
}

void KfLog::Output(const char* type, const char* file, const char* function, int line, const wchar_t* format, ...) {
	KfString log_header = ContactLogHeader(type, file, function, line).c_str();

	va_list list;
	va_start(list, format);

	auto log_content = KfString::FormatList(format, list);

	va_end(list);

	auto result = log_header + L" " + log_content.c_str() + L"\n";

	AppendData(out_file_name_.c_str(), result);
}

void KfLog::Output(const char* type, const char* file, const char* function, int line, const std::string& value) {
	KfString log_header = ContactLogHeader(type, file, function, line).c_str();

	auto result = log_header + L" " + value.c_str() + L"\n";

	AppendData(out_file_name_.c_str(), result);
}

void KfLog::Output(const char* type, const char* file, const char* function, int line, const std::wstring& value) {
	KfString log_header = ContactLogHeader(type, file, function, line).c_str();

	auto result = log_header + L" " + value.c_str() + L"\n";

	AppendData(out_file_name_.c_str(), result);
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

bool KfLog::HasErrorLog() {
	FileReadStream file_stream(out_file_name_.c_str());

	auto file_data = file_stream.Read();

	return std::string::npos != std::string(file_data.data()).find("[error ");
}

std::string KfLog::GetLogPath() {
	return out_file_name_;
}

bool KfLog::AppendData(const char* path, const char* data) {
	std::unique_lock<std::mutex> lock(mtx_);

	// OutputDebugStringA(data);
	// OutputDebugStringA("\n");

	std::cout << data << "\n";

	FILE* f;
	auto err = fopen_s(&f, path, "ab");
	if (f == nullptr) {
		DWORD dw_err = GetLastError();
		return false;
	}
	fwrite(data, strlen(data), 1, f);
	fclose(f);
	return true;
}

std::wstring KfLog::ContactLogHeader(const char* type, const char* file, const char* function, int line) {
	auto current_time = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(current_time);
	std::tm now;
	std::ignore = localtime_s(&now, &t);

	char error_info[MAX_PATH];
	ZeroMemory(error_info, sizeof(error_info));

	if (StrStrIA(type, "error")) {
		std::ignore = sprintf_s(error_info, MAX_PATH, "error code: %d", GetLastError());
	}

	return KfString::Format("[%s %04d-%02d-%02d %02d:%02d:%02d]%s:%d %s %s",
							type, now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec,
							PathFindFileNameA(file), line, function, error_info);
}











KfTimer::KfTimer(const std::string& value) {
	index_ = value;
	KF_INFO("start executable: %s", index_.c_str());
	start_time_ = GetTickCount();
}

KfTimer::KfTimer(const std::wstring& value) {
	index_ = CStringHelper::w2a(value);
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

	index_ = CStringHelper::w2a(value.get());
	KF_INFO("start executable: %s", index_.c_str());
	start_time_ = GetTickCount();
}

KfTimer::~KfTimer() {
	auto diff_time = GetTickCount() - start_time_;
	if (diff_time > time_out_) {
		KF_WARN("finished executable: %s, time out: %dms", index_.c_str(), diff_time);
		return;
	}

	KF_INFO("finished executable: %s, time: %dms", index_.c_str(), diff_time);
}
