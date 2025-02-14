#pragma once

#include <deque>
#include <map>
#include <thread>

#include "curl_download_request.h"

typedef std::function<int(void*, const char*, double)> DownloadProgressFunction;
typedef std::function<void(void*, const char*, const char*)> DownloadNotify;

struct CallbackInfo
{
	ptrDownloadFinishedCallback finished_callback;
	DownloadProgressFunction progress_callback;
	DownloadNotify notify_callback;
	void* user_ptr;
};

struct TaskInfo
{
	CallbackInfo callback_info;
	std::string url;

	bool accept_ranges = false;
	double size = 0.0f;
	std::wstring file_path;
};

struct DownloadInfo
{
	CallbackInfo callback_info;
	std::shared_ptr<CurlDownloadRequest> download;
};

class CurlDownloadManager {
public:
	static void SetDownloadCount(uint8_t value);

	static void AddTask(const char* url, const CallbackInfo& callback_info);

	static void AddTask(const char* url, bool accept_ranges, double size,
						const wchar_t* file_path,
						const CallbackInfo& callback_info);

	static void PauseTask(const char* url);

	static void ResumeTask(const char* url);

	static bool InDownloadList(const char* url);

	static bool IsPaused(const char* url);

	static void Quit();
protected:
	static void OnDownloadFinished(void* ptr_user, const char* sign,
								   std::wstring_view file_path, CURLcode code,
								   int http_code);

	static int OnProgressFunction(void* ptr, const char* sign,
								  double now_downloaded,
								  double total_to_download, int index,
								  long long now_download_size,
								  long long total_download_size);

	static int OnDownloadProgressSingleThread(void* clientp, const char* sign,
											  double dltotal, double dlnow,
											  double speed);

	static void OnNotifyCallback(void* user_ptr, const char* url, const char* msg);
private:
	static void DownloadTask(const char* url, const CallbackInfo& callback_info);

	static void DownloadTask(const char* url, bool accept_ranges,
							 double length, const wchar_t* file_path,
							 const CallbackInfo& callback_info);

	static void PushTask();

	inline static std::deque<TaskInfo> task_infos;
	inline static std::map<std::string, DownloadInfo> download_infos;

	inline static uint8_t thread_count_ = 3;

	inline static bool stop_download_ = false;

	inline static std::vector<std::thread> download_threads_;
};