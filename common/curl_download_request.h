#pragma once

#include <functional>
#include <curl/curl.h>

typedef std::function<int(void*, double, double, int, long long, long long)> ptrDownloadProgressFunction;
typedef std::function<void(void*, std::wstring_view)> ptrDownloadFinishedCallback;

typedef int (*ptrDownloadProgressSingleThreadFunction)(void* clientp,
													   double dltotal,
													   double dlnow,
													   double ultotal,
													   double ulnow);

struct DownloadNode
{
	long long start_pos;
	long long record_start_pos;
	long long end_pos;
	long long total_download_size;
	CURL* curl;
	FILE* file;
	uint8_t index;
	ptrDownloadProgressFunction download_progress_callback_ = nullptr;
	void* download_progress_callback_data_ = nullptr;
};

class CurlDownloadRequest
{
public:
	void SetUrl(const char* url);

	void SetDownloadProgressCallback(const ptrDownloadProgressFunction& callback, void* data);

	void SetDownloadFinishedCallback(const ptrDownloadFinishedCallback& callback, void* data);

	double GetContentLength(bool* accept_ranges, std::string* ptr_file_name) const;

	// 多线程下载
	long DownloadFile(double content_length, std::wstring_view target_file_path, unsigned thread_count);

	// 单线程下载
	bool DownloadSingleThreadFile(const wchar_t* target_file_path,
								  const ptrDownloadProgressSingleThreadFunction& progress_callback,
								  void* user_data);

	void StopDownload();

	bool IsStop() const;
protected:
	static size_t WriteDownloadFunction(void* buffer, size_t size, size_t nmemb, void* user_ptr);

	static size_t WriteSingleThreadDownloadFunction(void* buffer, size_t size, size_t nmemb, void* user_ptr);

	static int ProgressFunction(void* ptr, double total_to_download, double now_downloaded,
								double total_to_upload, double now_uploaded);
private:
	void Download(DownloadNode* node);
	
	std::string url_;

	ptrDownloadProgressFunction download_progress_callback_ = nullptr;
	void* download_progress_callback_data_ = nullptr;

	ptrDownloadFinishedCallback download_finished_callback_ = nullptr;
	void* download_finished_callback_data_ = nullptr;

	inline static bool stop_download_ = false;

	CURLcode download_result_code_ = CURLE_OK;
};