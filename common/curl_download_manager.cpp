#include "curl_download_manager.h"

#include <memory>

#include <Shlwapi.h>
#include <stdexcept>
#include <thread>

#include "curl_download_request.h"
#include "log_helper.h"

void CurlDownloadManager::SetDownloadCount(uint8_t value) {
	thread_count_ = value;
}

void CurlDownloadManager::AddTask(const char* url, const CallbackInfo& callback_info) {
	TaskInfo info;
	info.callback_info = callback_info;
	info.url = url;
	task_infos.emplace_back(info);
	callback_info.notify_callback(callback_info.user_ptr, url, "排队中...");
	PushTask();
}

void CurlDownloadManager::AddTask(const char* url, bool accept_ranges,
								  double size, const wchar_t* file_path,
								  const CallbackInfo& callback_info) {
	TaskInfo info;

	info.url = url;
	info.accept_ranges = accept_ranges;
	info.size = size;
	info.file_path = file_path;
	info.callback_info = callback_info;

	task_infos.emplace_back(info);

	callback_info.notify_callback(callback_info.user_ptr, url, "排队中...");

	PushTask();
}

void CurlDownloadManager::PauseTask(const char* url) {
	auto it_find = download_infos.find(url);
	if(it_find != download_infos.end()) {
		// it_find->second.download->Pause();
		throw std::runtime_error("not implement");
		it_find->second.callback_info.notify_callback(it_find->second.callback_info.user_ptr, url, "已暂停");
	}
}

void CurlDownloadManager::ResumeTask(const char* url) {
	auto it_find = download_infos.find(url);
	if (it_find != download_infos.end()) {
		// it_find->second.download->Resume();
		throw std::runtime_error("not implement");
		it_find->second.callback_info.notify_callback(it_find->second.callback_info.user_ptr,
													  url, "继续下载");
	}
}

bool CurlDownloadManager::InDownloadList(const char* url) {
	auto it_find = download_infos.find(url);
	if (it_find != download_infos.end()) {
		return true;
	}

	return false;
}

bool CurlDownloadManager::IsPaused(const char* url) {
	auto it_find = download_infos.find(url);
	if (it_find != download_infos.end()) {
		// return it_find->second.download->IsPaused();
		throw std::runtime_error("not implement");
	}

	return false;
}

void CurlDownloadManager::Quit() {
	stop_download_ = true;

	for (auto& it : download_infos) {
		it.second.download->StopDownload();
	}

	for (auto& it : download_threads_) {
		if (it.joinable()) {
			it.join();
		}
	}
}

void CurlDownloadManager::OnDownloadFinished(void* ptr_user, const char* sign,
											 std::wstring_view file_path,
											 CURLcode code, int http_code) {
	auto it_find = download_infos.find(sign);
	if (it_find != download_infos.end()) {
		it_find->second.callback_info.finished_callback(it_find->second.callback_info.user_ptr,
														sign, file_path.data(),
														code, http_code);
		download_infos.erase(it_find);
		KF_INFO("下载完成 %s 任务总数: %d", sign, download_infos.size());
		if (!stop_download_) {
			PushTask();
		}
	}
}

int CurlDownloadManager::OnProgressFunction(void* ptr, const char* sign,
											double now_downloaded,
											double total_to_download,
											int index,
											long long now_download_size,
											long long total_download_size) {
	auto it_find = download_infos.find(sign);
	if (it_find != download_infos.end()) {
		it_find->second.callback_info.progress_callback(it_find->second.callback_info.user_ptr,
														sign,
														now_downloaded / total_to_download);
	}

	return 0;
}

int CurlDownloadManager::OnDownloadProgressSingleThread(void* clientp,
														const char* sign,
														double dltotal,
														double dlnow,
														double speed) {
	auto it_find = download_infos.find(sign);
	if (it_find != download_infos.end()) {
		return it_find->second.callback_info.progress_callback(it_find->second.callback_info.user_ptr,
															   sign,
															   dlnow / dltotal);
	}

	return 0;
}

void CurlDownloadManager::OnNotifyCallback(void* user_ptr, const char* url, const char* msg) {
	auto it_find = download_infos.find(url);
	if(it_find != download_infos.end()) {
		it_find->second.callback_info.notify_callback(it_find->second.callback_info.user_ptr, url, msg);
	}
}

void CurlDownloadManager::DownloadTask(const char* url,
									   const CallbackInfo& callback_info) {
	auto download_request = std::make_shared<CurlDownloadRequest>();

	download_request->SetUrl(url);

	bool accept_ranges = false;
	std::wstring file_name;
	auto length = download_request->GetContentLength(&accept_ranges,
													 &file_name);

	wchar_t temp_path[MAX_PATH];
	ZeroMemory(temp_path, sizeof(temp_path));
	GetTempPath(MAX_PATH, temp_path);
	PathAppend(temp_path, file_name.c_str());

	DownloadTask(url, accept_ranges, length, temp_path, callback_info);
}

void CurlDownloadManager::DownloadTask(const char* url, bool accept_ranges,
									   double length, const wchar_t* file_path,
									   const CallbackInfo& callback_info) {
	auto download_request = std::make_shared<CurlDownloadRequest>();

	download_request->SetUrl(url);

	download_request->SetDownloadFinishedCallback(OnDownloadFinished,
												  callback_info.user_ptr,
												  url);

	if (accept_ranges && length > 0) {
		// 多线程下载
		download_request->SetDownloadProgressCallback(OnProgressFunction,
													  callback_info.user_ptr,
													  url);

		auto concurrency = std::thread::hardware_concurrency();
		download_request->DownloadFile(length, file_path, concurrency);
		return;
	}

	download_request->SetDownloadSingleProgressCallback(OnDownloadProgressSingleThread,
														callback_info.user_ptr,
														url);

	// 单线程下载
	download_request->DownloadSingleThreadFile(file_path);
}

void CurlDownloadManager::PushTask() {
	if (download_infos.size() >= thread_count_) {
		KF_INFO("已经添加到下载队列，等待中...");
		return;
	}

	if(task_infos.empty()) {
		KF_INFO("下载队列已清空");
		return;
	}

	auto front = task_infos.front();

	auto url = front.url;

	download_infos[url] = {
		front.callback_info,
		nullptr
	};

	front.callback_info.notify_callback(front.callback_info.user_ptr,
										front.url.c_str(), "开始下载");

	task_infos.pop_front();

	KF_INFO("开始下载 %s 任务总数: %d", url.c_str(), download_infos.size());

	std::thread download_thread([front] {
		if (!front.file_path.empty()) {
			DownloadTask(front.url.c_str(), front.accept_ranges, front.size,
						 front.file_path.c_str(), front.callback_info);
			return;
		}

		DownloadTask(front.url.c_str(), front.callback_info);
	});

	download_threads_.emplace_back(std::move(download_thread));
}
