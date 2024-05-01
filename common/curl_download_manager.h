#pragma once

#include <deque>
#include <functional>
#include <map>
#include <thread>

#include <curl/curl.h>

typedef std::function<int(void*, const char*, double, double, double)> DownloadProgress;
typedef std::function<void(void*, const char*, const char*, CURLcode, int)> DownloadFinished;
typedef std::function<void(void*, const char*, const char*)> DownloadNotify;

class CurlDownload {
public:
	void SetCallback(const DownloadProgress& progress_callback, const DownloadFinished& finished_callback,
					 const DownloadNotify& notify_callback, void* user_ptr);

	void Download(const char* url, const char* folder_path, const char* default_file_name);

	void Pause();

	void Resume();

	void Stop();

	bool IsPaused() const;
protected:
	static size_t WriteFunction(void* buff, size_t size, size_t element_count, void* user_ptr);

	static int ProgressFunction(void* ptr, double total_download, double now_download, double, double);
private:
	DownloadProgress progress_callback_;
	DownloadFinished finished_callback_;
	DownloadNotify notify_callback_;
	void* user_ptr_ = nullptr;

	std::string url_;

	CURL* curl_ = nullptr;

	bool stop_download_ = false;
	bool pause_download_ = false;
};










struct CallbackInfo
{
	DownloadProgress progress_callback;
	DownloadFinished finished_callback;
	DownloadNotify notify_callback;
	void* user_ptr;
};

struct TaskInfo
{
	CallbackInfo callback_info;
	std::string url;
};

struct DownloadInfo
{
	CallbackInfo callback_info;
	std::shared_ptr<CurlDownload> download;
};

class CurlDownloadManager {
public:
	static void SetDownloadCount(uint8_t value);

	static void AddTask(const char* url, const DownloadProgress& progress_callback,
						const DownloadFinished& finished_callback, const DownloadNotify& notify_callback,
						void* user_ptr);

	static void PauseTask(const char* url);

	static void ResumeTask(const char* url);

	static bool InDownloadList(const char* url);

	static bool IsPaused(const char* url);

	static void Quit();
protected:
	static int OnProgressCallback(void* user_ptr, const char* url,
								  double total_download, double now_download, double speed);

	static void OnFinishedCallback(void* user_ptr, const char* url, const char* file_path,
								   CURLcode code, int http_code);

	static void OnNotifyCallback(void* user_ptr, const char* url, const char* msg);
private:
	static void PushTask();

	inline static std::deque<TaskInfo> task_infos;
	inline static std::map<std::string, DownloadInfo> download_infos;

	inline static uint8_t thread_count_ = 3;

	inline static bool stop_download_ = false;

	inline static std::vector<std::thread> download_threads_;
};