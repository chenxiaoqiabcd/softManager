#include "curl_download_manager.h"

#include <cstdio>
#include <memory>

#include <Shlwapi.h>
#include <thread>

#include "log_helper.h"
#include "kf_str.h"
#include "stringHelper.h"

void CurlDownload::SetCallback(const DownloadProgress& progress_callback,
                               const DownloadFinished& finished_callback,
							   const DownloadNotify& notify_callback, void* user_ptr) {
	progress_callback_ = progress_callback;
	finished_callback_ = finished_callback;
	notify_callback_ = notify_callback;
	user_ptr_ = user_ptr;
}

std::string ParseFileName(const char* url) {
	KfString file_name = url;

	auto index = file_name.Find("?");
	if (std::string::npos != index) {
		file_name = file_name.Left(static_cast<int>(index));
	}

	index = file_name.ReverseFind("/");
	if (std::string::npos != index) {
		file_name = file_name.SubStr(static_cast<int>(index) + 1);
		return CStringHelper::DeescapeURL(file_name.GetString());
	}

	return "";
}

void CurlDownload::Download(const char* url, const char* folder_path, const char* default_file_name) {
	url_ = url;

	auto file_name = ParseFileName(url);

	if(file_name.empty() && nullptr != default_file_name) {
		file_name = default_file_name;
	}

	auto file_path = std::shared_ptr<char>(new char[MAX_PATH]);
	ZeroMemory(file_path.get(), sizeof(file_path));
	strcpy_s(file_path.get(), MAX_PATH, folder_path);
	PathAppendA(file_path.get(), file_name.c_str());

	auto file = fopen(file_path.get(), "wb");
	if(nullptr == file) {
		finished_callback_(user_ptr_, url, file_path.get(), CURLE_OK, 0);

		KF_ERROR("failed open file: %s", file_path.get());
		return;
	}

	curl_ = curl_easy_init();

	curl_easy_setopt(curl_, CURLOPT_URL, url);

	// 如果在5秒内低于1个字节/秒，则终止
	curl_easy_setopt(curl_, CURLOPT_LOW_SPEED_TIME, 60L);
	curl_easy_setopt(curl_, CURLOPT_LOW_SPEED_LIMIT, 30L);

	curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0);

	curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);

	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteFunction);

	curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl_, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(curl_, CURLOPT_PROGRESSFUNCTION, ProgressFunction);

	auto code = curl_easy_perform(curl_);

	int http_code = 0;
	curl_easy_getinfo(curl_, CURLINFO_HTTP_CODE, &http_code);

	std::ignore = fclose(file);

	curl_easy_cleanup(curl_);

	finished_callback_(user_ptr_, url, file_path.get(), code, http_code);
}

void CurlDownload::Pause() {
	pause_download_ = true;
	KF_INFO("暂停下载 url: %s", url_.c_str());

	curl_easy_pause(curl_, CURLPAUSE_RECV | CURLPAUSE_SEND);
}

void CurlDownload::Resume() {
	pause_download_ = false;
	KF_INFO("继续下载 url: %s", url_.c_str());
	curl_easy_pause(curl_, CURLPAUSE_CONT);
}

void CurlDownload::Stop() {
	stop_download_ = true;
}

bool CurlDownload::IsPaused() const {
	return pause_download_;
}

size_t CurlDownload::WriteFunction(void* buff, size_t size, size_t element_count, void* user_ptr) {
	std::ignore = fwrite(buff, size, element_count, static_cast<FILE*>(user_ptr));
	return size * element_count;
}

int CurlDownload::ProgressFunction(void* ptr, double total_download, double now_download, double, double) {
	auto pThis = static_cast<CurlDownload*>(ptr);

	if(pThis->stop_download_) {
		KF_INFO("取消下载 url: %s", pThis->url_.c_str());
		return -1;
	}

	double speed = 0.0;
	curl_easy_getinfo(pThis->curl_, CURLINFO_SPEED_DOWNLOAD, &speed);

	return pThis->progress_callback_(pThis->user_ptr_, pThis->url_.c_str(), total_download, now_download,
									 speed);
}











void CurlDownloadManager::SetDownloadCount(uint8_t value) {
	thread_count_ = value;
}

void CurlDownloadManager::AddTask(const char* url, const DownloadProgress& progress_callback,
								  const DownloadFinished& finished_callback,
								  const DownloadNotify& notify_callback, void* user_ptr) {
	TaskInfo info;
	info.callback_info.progress_callback = progress_callback;
	info.callback_info.finished_callback = finished_callback;
	info.callback_info.notify_callback = notify_callback;
	info.callback_info.user_ptr = user_ptr;
	info.url = url;

	task_infos.emplace_back(info);

	info.callback_info.notify_callback(user_ptr, url, "排队中...");

	PushTask();
}

void CurlDownloadManager::PauseTask(const char* url) {
	auto it_find = download_infos.find(url);
	if(it_find != download_infos.end()) {
		it_find->second.download->Pause();
		it_find->second.callback_info.notify_callback(it_find->second.callback_info.user_ptr, url, "已暂停");
	}
}

void CurlDownloadManager::ResumeTask(const char* url) {
	auto it_find = download_infos.find(url);
	if (it_find != download_infos.end()) {
		it_find->second.download->Resume();
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
		return it_find->second.download->IsPaused();
	}

	return false;
}

void CurlDownloadManager::Quit() {
	stop_download_ = true;

	for (auto& it : download_infos) {
		it.second.download->Stop();
	}

	for (auto& it : download_threads_) {
		if (it.joinable()) {
			it.join();
		}
	}
}

int CurlDownloadManager::OnProgressCallback(void* user_ptr, const char* url, double total_download,
											double now_download, double speed) {
	auto it_find = download_infos.find(url);
	if (it_find != download_infos.end()) {
		return it_find->second.callback_info.progress_callback(it_find->second.callback_info.user_ptr, url,
															   total_download, now_download, speed);
	}

	return 0;
}

void CurlDownloadManager::OnFinishedCallback(void* user_ptr, const char* url, const char* file_path,
											 CURLcode code, int http_code) {
	auto it_find = download_infos.find(url);
	if (it_find != download_infos.end()) {
		it_find->second.callback_info.finished_callback(it_find->second.callback_info.user_ptr, url,
														file_path, code, http_code);

		download_infos.erase(it_find);

		KF_INFO("下载完成 %s 任务总数: %d", url, download_infos.size());

		if (!stop_download_) {
			PushTask();
		}
	}
}

void CurlDownloadManager::OnNotifyCallback(void* user_ptr, const char* url, const char* msg) {
	auto it_find = download_infos.find(url);
	if(it_find != download_infos.end()) {
		it_find->second.callback_info.notify_callback(it_find->second.callback_info.user_ptr, url, msg);
	}
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
		{
			front.callback_info.progress_callback,
			front.callback_info.finished_callback,
			front.callback_info.notify_callback,
			front.callback_info.user_ptr,
		},
		nullptr
	};

	front.callback_info.notify_callback(front.callback_info.user_ptr, front.url.c_str(), "开始下载");

	task_infos.pop_front();

	KF_INFO("开始下载 %s 任务总数: %d", url.c_str(), download_infos.size());

	char temp_path [MAX_PATH] ;
	ZeroMemory(temp_path, sizeof(temp_path));
	GetTempPathA(MAX_PATH, temp_path);

	std::thread download_thread([url, temp_path] {
		auto download = std::make_shared<CurlDownload>();

		download_infos[url].download = download;

		download->SetCallback(OnProgressCallback, OnFinishedCallback, OnNotifyCallback, nullptr);
		download->Download(url.c_str(), temp_path, nullptr);
	});

	download_threads_.emplace_back(std::move(download_thread));
}
