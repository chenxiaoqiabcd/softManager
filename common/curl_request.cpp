#include "curl_request.h"

#include <Shlwapi.h>

#include "log_helper.h"
#include "kf_str.h"
#include "request_helper.h"

struct HeaderData
{
	FILE* file = nullptr;
	bool stop_ = false;
	std::wstring folder;
	std::wstring default_name;
	std::wstring file_path;
	std::string localtion;

	HeaderData(const wchar_t* folder, const wchar_t* default_name) {
		this->file = nullptr;
		this->folder = folder;
		this->default_name = default_name;
	}
};

size_t WriteFunction(void* buffer, size_t size, size_t nmemb, void* user_ptr) {
	auto header_data = static_cast<HeaderData*>(user_ptr);

	if(nullptr == header_data->file) {
		wchar_t file_path[MAX_PATH];
		ZeroMemory(file_path, sizeof(wchar_t) * MAX_PATH);
		wcscpy_s(file_path, header_data->folder.c_str());
		PathAppend(file_path, header_data->default_name.c_str());

		header_data->file_path = file_path;

		header_data->file = _wfopen(file_path, L"wb");
	}

	fwrite(buffer, size, nmemb, header_data->file);
	return size * nmemb;
}

auto WriteHeader(void* ptr, size_t size, size_t nmemb, void* stream) -> size_t {
	auto header_data = static_cast<HeaderData*>(stream);

	char head[2048];
	ZeroMemory(head, sizeof(head));
	memcpy(head, ptr, size * nmemb + 1);

	KF_INFO("response header: %s", head);

	std::string filter = "Content-Disposition: ";

	KfString temp(head);
	auto index = temp.Find(filter.c_str());
	const auto value_index = temp.Find("=");
	if (index != std::string::npos && value_index != std::string::npos) {

		auto file_name = temp.SubStr(value_index + 1).Replace("\r\n", "");

		wchar_t file_path[MAX_PATH];
		ZeroMemory(file_path, sizeof(wchar_t) * MAX_PATH);
		wcscpy_s(file_path, header_data->folder.c_str());
		PathAppend(file_path, file_name.GetWString().c_str());

		header_data->file_path = file_path;

		header_data->file = _wfopen(file_path, L"wb");
	}

	filter = "location: ";

	index = temp.MakeLower().Find(filter.c_str());
	if (index != std::string::npos) {
		header_data->localtion = temp.SubStr(index + filter.length()).Replace("\r\n", "");
	}

	return size * nmemb;
}

CurlRequest::CurlRequest() {
	curl_ = curl_easy_init();

	curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0);
}

CurlRequest::~CurlRequest() {
	curl_easy_cleanup(curl_);
}

void CurlRequest::SetUrl(std::string_view url) const {
	if (nullptr == StrStrIA(url.data(), "http")) {
		curl_easy_setopt(curl_, CURLOPT_URL, RequestHelper::GetHost() + url.data());
		return;
	}

	curl_easy_setopt(curl_, CURLOPT_URL, url.data());
}

void CurlRequest::SetDownloadFilePath(std::wstring_view dest_file_path) {
	FILE* fp = nullptr;
	header_data_ = new HeaderData(dest_file_path.data(), L"1.exe");
}

void CurlRequest::SetDownloadProgressCallback(ptrProgressFunction callback, void* data) {
	download_progress_callback_ = callback;

	curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl_, CURLOPT_PROGRESSDATA, data);
	curl_easy_setopt(curl_, CURLOPT_PROGRESSFUNCTION, callback);
}

void CurlRequest::SetDownloadFinishedCallback(ptrFinishedCallback callback, void* data) {
	download_finished_callback_ = callback;
	data_ = data;
}

long CurlRequest::DownloadFile() {
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, header_data_);
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteFunction);

	curl_easy_setopt(curl_, CURLOPT_HEADERDATA, header_data_);
	curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, WriteHeader);

	if (nullptr == download_progress_callback_) {
		curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 1);
	}

	const CURLcode res = curl_easy_perform(curl_);

	long response_code;
	curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code);

	if(nullptr != header_data_->file)
		std::ignore = fclose(header_data_->file);

	if (nullptr != download_finished_callback_ && 200 == response_code) {
		download_finished_callback_(res, data_, header_data_->file_path);

		delete header_data_;
		header_data_ = nullptr;
	}

	return response_code;
}

long CurlRequest::ReDownloadFile() {
	SetUrl(header_data_->localtion);
	return DownloadFile();
}

size_t WriteResponseFunction(void* ptr, size_t size, size_t nmemb, void* userdata) {
	const auto response_body = static_cast<std::string*>(userdata);
	response_body->append(static_cast<char*>(ptr), size * nmemb);
	return size * nmemb;
}

bool CurlRequest::DoPost(const char* request_body, std::string* ptr_response_body) const {
	curl_easy_setopt(curl_, CURLOPT_POST, 1L);
	curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, request_body);

	struct curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type:application/json");

	curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, ptr_response_body);
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteResponseFunction);

	const CURLcode res = curl_easy_perform(curl_);

	return res == CURLE_OK;
}
