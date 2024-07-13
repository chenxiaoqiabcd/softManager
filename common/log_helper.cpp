#include "log_helper.h"

#include <chrono>
#include <filesystem>
#include <iostream>

#include <Shlwapi.h>

#include "file_helper.h"
#include "file_read_stream.h"
#include "helper.h"
#include "kf_str.h"
#include "stringHelper.h"

#define MAX_LEN MAX_PATH

void KfLog::EnableLocalLog() {
	auto folder = std::filesystem::path(Helper::GetRoamingDir()) / "softManager";

	std::error_code error_code;

	if(!std::filesystem::is_directory(folder)) {
		std::filesystem::create_directories(folder, error_code);
	}

	out_file_name_ = (folder / "log.data").string();
}

void KfLog::ClearLocalLog() {
	DeleteFileA(out_file_name_.c_str());
}

void KfLog::Output(const char* type, const char* file, const char* function, int line) {
	auto current_time = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(current_time);
	std::tm* now = std::localtime(&t);

	char error_info[MAX_PATH];
	ZeroMemory(error_info, sizeof(error_info));

	if (StrStrIA(type, "error")) {
		sprintf(error_info, "error code: %d", GetLastError());
	}

	char data[1024];
	ZeroMemory(data, 1024);
	sprintf_s(data, "[%s %02d:%02d:%02d]%s:%d %s %s ", type, now->tm_hour, now->tm_min, now->tm_sec,
			  PathFindFileNameA(file), line, function, error_info);

	if (!out_file_name_.empty()) {
		AppendData(out_file_name_.c_str(), data);
	}

	std::cout << data;
}

void KfLog::Output(char const* const format, ...) {
	va_list list;
	va_start(list, format);

	std::string result = KfString::FormatList(format, list);

	va_end(list);

	result.append("\n");

	if (!out_file_name_.empty()) {
		AppendData(out_file_name_.c_str(), result.c_str());
	}

	std::cout << result;
}

void KfLog::Output(const wchar_t* format, ...) {
	va_list list;
	va_start(list, format);

	const std::wstring result = KfString::FormatList(format, list);

	va_end(list);

	const auto data = CStringHelper::w2a(result) + "\n";

	if (!out_file_name_.empty()) {
		AppendData(out_file_name_.c_str(), data.c_str());
	}

	std::cout << data;
}

void KfLog::Output(const std::string& value) {
	std::string data = value;
	data.append("\n");

	if (!out_file_name_.empty()) {
		AppendData(out_file_name_.c_str(), data.c_str());
	}

	std::cout << data;
}

void KfLog::Output(const std::wstring& value) {
	std::string data = CStringHelper::w2a(value);
	data.append("\n");

	if (!out_file_name_.empty()) {
		AppendData(out_file_name_.c_str(), data.c_str());
	}

	std::cout << data;
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


void logTest() {
	KF_TIMER(L"%s", __FUNCTIONW__);

	KF_INFO("hello %s", "world");
	KF_WARN("hello %s", "world");
	KF_ERROR("hello %s", "world");
}
