#pragma once

#include <memory>
#include <string_view>

#include "WndImpl.h"

class Scheme;
class CurlDownloadRequest;

class MainWindow : public CWndImpl
{
protected:
	LPCTSTR GetSkinFile() override;

	void Init() override;

	void Notify(DuiLib::TNotifyUI& msg) override;

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	static void OnFinishedCallback(void* ptr, std::wstring_view file_path);

	static int OnProgressSingleThreadCallback(void* ptr, double total_to_download,
											  double now_downloaded, double, double);

	static int OnProgressFunctionV2(void* ptr, double now_downloaded, double total_to_download,
									int index, long long now_download_size,
									long long total_download_size);
private:
	bool DownloadTask(const char* url, const wchar_t* dest_folder);

	std::shared_ptr<CurlDownloadRequest> download_request_;

	std::shared_ptr<Scheme> scheme_;
};
