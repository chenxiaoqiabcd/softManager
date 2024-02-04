#include "curl_download_request.h"

#include <map>
#include <mutex>
#include <Shlwapi.h>
#include <thread>

#include "helper.h"
#include "kf_log.h"
#include "kf_str.h"
#include "request_helper.h"
#include "stringHelper.h"

void CurlDownloadRequest::SetUrl(const char* url) {
	if (nullptr == StrStrIA(url, "http")) {
		url_ = RequestHelper::GetHost() + url;
		return;
	}

	url_ = url;
}

void CurlDownloadRequest::SetDownloadProgressCallback(const ptrDownloadProgressFunction& callback,
													  void* data) {
	download_progress_callback_ = callback;
	download_progress_callback_data_ = data;
}

void CurlDownloadRequest::SetDownloadSingleProgressCallback(const ptrDownloadProgressSingleThreadFunction& callback, void* data) {
	download_progress_single_thread_callback_ = callback;
	download_progress_single_thread_callback_data_ = data;
}

void CurlDownloadRequest::SetDownloadFinishedCallback(const ptrDownloadFinishedCallback& callback,
													  void* data) {
	download_finished_callback_ = callback;
	download_finished_callback_data_ = data;
}

struct HeaderData
{
	bool accept_ranges = false;
	std::string file_name;
};

auto OutputHeader(void* ptr, size_t size, size_t nmemb, void* stream) -> size_t {
	const auto header_data = static_cast<HeaderData*>(stream);

	std::shared_ptr<char> header(new char[size * nmemb + 1]);
	ZeroMemory(header.get(), size * nmemb + 1);
	memcpy(header.get(), ptr, size * nmemb + 1);

	auto filter = "Accept-Ranges: ";
	if (StrStrIA(header.get(), filter)) {
		header_data->accept_ranges = true;

		KF_INFO("out put header: %s", header.get());
	}

	KfString temp(header.get());

	filter = "Content-Disposition: ";

	auto index = temp.Find(filter);
	const auto value_index = temp.Find("=");
	if (index != std::string::npos && value_index != std::string::npos) {
		KfString file_name = temp.SubStr(static_cast<int>(value_index) + 1);
		file_name = CStringHelper::DeescapeURL(file_name.GetString()).c_str();
		header_data->file_name = file_name.Replace("*", "").Replace("\r\n", "").Replace("\"", "");

		KF_INFO("out put header: %s", header.get());
	}

	filter = "location: ";
	index = temp.MakeLower().Find(filter);
	if (std::string::npos != index) {
		temp = temp.SubStr(index + strlen(filter));
		index = temp.Find("?");
		if(index != std::string::npos) {
			temp = temp.SubStr(0, index);
			index = temp.ReverseFind("/");
			if(std::string::npos != index) {
				header_data->file_name = temp.SubStr(index + 1);

				KF_INFO_L(nmemb * size + 17, "out put header: %s", header.get());
			}
		}
	}

	return size * nmemb;
}

double CurlDownloadRequest::GetContentLength(bool* accept_ranges, std::string* ptr_file_name) const {
	auto ReleaseCurl = [](CURL* c) {
		curl_easy_cleanup(c);
	};

	std::unique_ptr<CURL, decltype(ReleaseCurl)> curl_guard(curl_easy_init(), ReleaseCurl);

	curl_easy_setopt(curl_guard.get(), CURLOPT_URL, url_.c_str());

	KfString file_name = url_.c_str();
		
	auto index = file_name.Find("?");
	if (std::string::npos != index) {
		file_name = file_name.Left(static_cast<int>(index));
	}

	index = file_name.ReverseFind("/");
	if (std::string::npos != index) {
		file_name = file_name.SubStr(static_cast<int>(index) + 1);
		*ptr_file_name = CStringHelper::DeescapeURL(file_name.GetString());
	}

	// 如果在5秒内低于1字节/秒，则终止
	curl_easy_setopt(curl_guard.get(), CURLOPT_LOW_SPEED_TIME, 1L);
	curl_easy_setopt(curl_guard.get(), CURLOPT_LOW_SPEED_LIMIT, 1L);

	curl_easy_setopt(curl_guard.get(), CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl_guard.get(), CURLOPT_SSL_VERIFYHOST, 0);

	curl_easy_setopt(curl_guard.get(), CURLOPT_FOLLOWLOCATION, 1L);

	const auto header_data = std::make_shared<HeaderData>();

	curl_easy_setopt(curl_guard.get(), CURLOPT_HEADER, 1L);
	curl_easy_setopt(curl_guard.get(), CURLOPT_HEADERDATA, header_data.get());
	curl_easy_setopt(curl_guard.get(), CURLOPT_HEADERFUNCTION, OutputHeader);

	curl_easy_setopt(curl_guard.get(), CURLOPT_NOPROGRESS, 1L);

	curl_easy_setopt(curl_guard.get(), CURLOPT_NOBODY, 1L);

	const CURLcode code = curl_easy_perform(curl_guard.get());

	// CURLE_WRITE_ERROR 是正常的，因为我们只需要获取头部信息
	if (code == CURLE_OK || code == CURLE_WRITE_ERROR || code == CURLE_RECV_ERROR) {
		*accept_ranges = header_data->accept_ranges;

		if(!header_data->file_name.empty()) {
			*ptr_file_name = header_data->file_name;
		}

		double file_length = 0.0;
		curl_easy_getinfo(curl_guard.get(), CURLINFO_CONTENT_LENGTH_DOWNLOAD, &file_length);
		KF_INFO("success get file length: %s", Helper::ToStringSize(file_length).c_str());
		return file_length;
	}

	KF_WARN("failed get file length: error code: %d", code);
	return -1;
}

std::map<int, long long> download_map_;

long CurlDownloadRequest::DownloadFile(double content_length, std::wstring_view target_file_path, 
									   unsigned thread_count) {
	// 重置状态值
	stop_download_ = false;
	download_result_code_ = CURLE_OK;
	download_map_.clear();

	if(PathFileExists(target_file_path.data())) {
		DeleteFile(target_file_path.data());
	}

	FILE* file = _wfopen(target_file_path.data(), L"wb");
	if (nullptr == file) {
		KF_WARN(L"failed open file: %s", target_file_path.data());
		return -1;
	}

	std::vector<std::thread> download_threads;

	for (unsigned n = 0; n < thread_count; ++n) {
		auto node = new DownloadNode;

		node->total_download_size = content_length;
		node->download_progress_callback_ = download_progress_callback_;
		node->download_progress_callback_data_ = download_progress_callback_data_;
		node->record_start_pos = node->start_pos = content_length / thread_count * n;
		node->end_pos = content_length / thread_count * (n + 1) - 1;

		if (n == thread_count - 1) {
			node->end_pos = content_length;
		}

		KF_INFO("thread: %d, start: %lld, end: %lld, size: %s", n + 1, node->start_pos, node->end_pos,
				Helper::ToStringSize(node->end_pos - node->start_pos).c_str());

		auto curl = curl_easy_init();

		node->curl = curl;
		node->file = file;
		node->index = n + 1;

		curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());

		// 如果在5秒内低于1个字节/秒，则终止
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 5L);
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		std::string range;
		range.append(std::to_string(node->start_pos));
		range.append("-");
		range.append(std::to_string(node->end_pos));

		curl_easy_setopt(curl, CURLOPT_RANGE, range.c_str());

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, node);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteDownloadFunction);

		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, node);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressFunction);

		std::thread download_thread([this, node] {
			Download(node);
		});

		download_threads.emplace_back(std::move(download_thread));
	}

	for(auto& it : download_threads) {
		if(!it.joinable()) {
			continue;
		}

		it.join();
	}

	std::ignore = fclose(file);

	if(stop_download_) {
		return download_result_code_;
	}

	if(nullptr != download_finished_callback_) {
		if (download_result_code_ == CURLE_ABORTED_BY_CALLBACK || download_result_code_ == CURLE_BAD_FUNCTION_ARGUMENT) {
			// 取消下载
			KF_INFO("cancel download, url: %s", url_.c_str());
			return download_result_code_;
		}

		download_finished_callback_(download_finished_callback_data_, target_file_path);
	}

	return download_result_code_;
}

bool CurlDownloadRequest::DownloadSingleThreadFile(const wchar_t* target_file_path) {
	// 重置状态值
	stop_download_ = false;
	download_result_code_ = CURLE_OK;
	download_map_.clear();

	if (PathFileExists(target_file_path)) {
		DeleteFile(target_file_path);
	}

	FILE* file = _wfopen(target_file_path, L"wb");
	if (nullptr == file) {
		KF_WARN(L"failed open file: %s", target_file_path);
		return false;
	}

	single_curl_ = curl_easy_init();

	curl_easy_setopt(single_curl_, CURLOPT_URL, url_.c_str());

	// 如果在5秒内低于1个字节/秒，则终止
	curl_easy_setopt(single_curl_, CURLOPT_LOW_SPEED_TIME, 60L);
	curl_easy_setopt(single_curl_, CURLOPT_LOW_SPEED_LIMIT, 30L);

	curl_easy_setopt(single_curl_, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(single_curl_, CURLOPT_SSL_VERIFYHOST, 0);

	curl_easy_setopt(single_curl_, CURLOPT_FOLLOWLOCATION, 1L);

	curl_easy_setopt(single_curl_, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(single_curl_, CURLOPT_WRITEFUNCTION, WriteSingleThreadDownloadFunction);

	curl_easy_setopt(single_curl_, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(single_curl_, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(single_curl_, CURLOPT_PROGRESSFUNCTION, SingleProcessProgressFunction);

	const auto code = curl_easy_perform(single_curl_);

	std::ignore = fclose(file);

	if(stop_download_) {
		return false;
	}

	curl_easy_cleanup(single_curl_);

	if (CURLE_OK == code) {
		if (nullptr != download_finished_callback_) {
			if (download_result_code_ == CURLE_ABORTED_BY_CALLBACK || download_result_code_ == CURLE_BAD_FUNCTION_ARGUMENT) {
				// 取消下载
				KF_INFO("cancel download, url: %s, code: %d", url_.c_str(), code);
				return code == CURLE_OK;
			}

			download_finished_callback_(download_finished_callback_data_, target_file_path);
		}

		KF_INFO("success download, url: %s", url_.c_str());
		return true;
	}

	KF_WARN("failed download, url: %s, code: %d", url_.c_str(), code);
	return false;
}

void CurlDownloadRequest::StopDownload() {
	stop_download_ = true;
}

bool CurlDownloadRequest::IsStop() const {
	return stop_download_;
}

std::mutex mtx;

size_t CurlDownloadRequest::WriteDownloadFunction(void* buffer, size_t size, size_t nmemb, void* user_ptr) {
	const auto node = static_cast<DownloadNode*>(user_ptr);

	const size_t written = size * nmemb;

	mtx.lock();

	std::ignore = fseek(node->file, node->start_pos, SEEK_SET);

	if (node->start_pos + written <= node->end_pos) {
		std::ignore = fwrite(buffer, size, nmemb, node->file);
		node->start_pos += written;
	}
	else {
		std::ignore = fwrite(buffer, 1, node->end_pos - node->start_pos + 1, node->file);
		node->start_pos = node->end_pos;
	}

	mtx.unlock();

	return written;
}

size_t CurlDownloadRequest::WriteSingleThreadDownloadFunction(void* buffer, size_t size, size_t nmemb,
															  void* user_ptr) {
	const auto file = static_cast<FILE*>(user_ptr);

	std::ignore = fwrite(buffer, size, nmemb, file);

	return size * nmemb;
}

int CurlDownloadRequest::ProgressFunction(void* ptr, double total_to_download, double now_downloaded,
										  double total_to_upload, double now_uploaded) {
	int result = 0;

	if(stop_download_) {
		return -1;
	}

	if (total_to_download > 0 && now_downloaded > 0) {
		mtx.lock();

		auto node = static_cast<DownloadNode*>(ptr);

		double speed;
		curl_easy_getinfo(node->curl, CURLINFO_SPEED_DOWNLOAD, &speed);

		long long total_download_size = 0;

		for (const auto& it : download_map_) {
			total_download_size += it.second;
		}

		const auto total = node->end_pos - node->record_start_pos;

		auto pos = static_cast<double>(total) - total_to_download + now_downloaded;

		const auto it_find = download_map_.find(node->index);
		if (it_find != download_map_.end() && pos > it_find->second) {
			KF_INFO("download index: %2d, progress: %3.2lf%% %10s/%10s %6.2lf%% speed: %10s/s",
					node->index, total_download_size * 100.0 / node->total_download_size,
					Helper::ToStringSize(now_downloaded).c_str(),
					Helper::ToStringSize(total_to_download).c_str(),
					now_downloaded * 100.0 / total_to_download,
					Helper::ToStringSize(speed).c_str());
		}

		download_map_[node->index] = pos;

		if (nullptr != node->download_progress_callback_) {
			result = node->download_progress_callback_(node->download_progress_callback_data_,
													   total_download_size, node->total_download_size,
													   node->index, pos, total);
		}

		mtx.unlock();
	}

	return result;
}

int CurlDownloadRequest::SingleProcessProgressFunction(void* ptr, double total_to_download,
													   double now_downloaded, double total_to_upload,
													   double now_uploaded) {
	int result = 0;

	if(stop_download_) {
		return -1;
	}

	if(total_to_download > 0 && now_downloaded > 0) {
		const auto pThis = static_cast<CurlDownloadRequest*>(ptr);

		double speed;
		curl_easy_getinfo(pThis->single_curl_, CURLINFO_SPEED_DOWNLOAD, &speed);

		if(nullptr != pThis->download_progress_single_thread_callback_) {
			result = pThis->download_progress_single_thread_callback_(pThis->download_progress_single_thread_callback_data_,
																	  total_to_download, now_downloaded, speed);
		} 
	}

	return result;
}

void CurlDownloadRequest::Download(DownloadNode* node) {
	auto code = curl_easy_perform(node->curl);

	if (CURLE_OK == code) {
		KF_INFO("success download index: %d", node->index);
	}
	else {
		KF_WARN("failed download index: %d, code: %d", node->index, code);
	}

	uint8_t retries = 1;

	while (CURLE_OK != code) {
		KF_INFO("retry download thread: %d, start: %lld, end: %lld, size: %s, retry count: %d",
				node->index, node->start_pos, node->end_pos,
				Helper::ToStringSize(node->end_pos - node->start_pos).c_str(), retries++);

		std::string range;
		range.append(std::to_string(node->start_pos));
		range.append("-");
		range.append(std::to_string(node->end_pos));

		curl_easy_setopt(node->curl, CURLOPT_RANGE, range.c_str());

		code = curl_easy_perform(node->curl);

		if (CURLE_OK == code) {
			KF_INFO("success download index: %d", node->index);
			break;
		}

		download_result_code_ = code;

		if (CURLE_ABORTED_BY_CALLBACK == code) {
			KF_INFO("abort download");
			break;
		}

		KF_WARN("failed download index: %d, code: %d", node->index, code);
	}

	curl_easy_cleanup(node->curl);

	delete node;
	node = nullptr;
}