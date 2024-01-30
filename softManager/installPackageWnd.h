#pragma once
#include <memory>

#include "WndImpl.h"

#include <string_view>

class CurlDownloadRequest;

class CInstallPackageWnd : public CWndImpl {
public:
	void SetInfo(const wchar_t* url, const std::vector<std::map<std::wstring, std::wstring>>& actions);
protected:
	LPCTSTR GetSkinFile() override;

	void Init() override;

	void NotifyClicked(const wchar_t* name);

	void Notify(DuiLib::TNotifyUI& msg) override;

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	static void OnFinishedCallback(void* ptr, std::wstring_view file_path);

	static DWORD WINAPI ThreadDelayInit(LPVOID lParam);

	static int OnProgressMultipleThreadCallback(void* ptr, double now_downloaded, double total_to_download,
												int index, long long now_download_size,
												long long total_download_size);

	static int OnProgressSingleThreadCallback(void* ptr, double total_to_download, double now_downloaded,
											  double, double);
private:
	bool DownLoad(std::string_view url, std::wstring_view temp_path);

	bool DownloadMultiThread(std::wstring_view file_path, double file_length);

	std::string url_;

	std::vector<std::map<std::wstring, std::wstring>> actions_;

	std::shared_ptr<CurlDownloadRequest> download_request_;
};
