#pragma once

#include <string_view>

#include <UIlib.h>

#include <curl/curl.h>

class Scheme;

class SoftListOperatorNode : public DuiLib::CHorizontalLayoutUI, public DuiLib::INotifyUI {
public:
	~SoftListOperatorNode() override;

	void SetScheme(Scheme* scheme);

	void SetMessage(const wchar_t* value);

	void SetKeyName(const wchar_t* value);

	void SetUninstallPath(const wchar_t* value);

	void SetLocalVersion(const char* value);

	void SetUpdateInfo(const char* last_version, const char* download_url,
					   const std::vector<std::map<std::wstring, std::wstring>>& actions, bool cracked);

	void UpdateMessage(const wchar_t* value) const;

	void UpdateUpgradeInfo(const char* last_version, const char* download_url);
protected:
	void Init() override;

	void NotifyClickedUpdateButton(DuiLib::TNotifyUI& msg);

	void Notify(DuiLib::TNotifyUI& msg) override;

	static int OnDownloadProgress(void* user_ptr, const char* url,
								  double total_download, double now_download, double speed);

	static void OnDownloadFinished(void* user_ptr, const char* url, const char* file_path,
								   CURLcode code, int http_code);

	static void OnDownloadNotify(void* user_ptr, const char* url, const char* msg);
private:
	DuiLib::CButtonUI* CreateUninstallButton() const;

	DuiLib::CButtonUI* CreateUpdateButton() const;

	DuiLib::CLabelUI* CreateInfoLabel() const;

	void Uninstall(std::wstring_view cmd);

	DuiLib::CButtonUI* btn_uninstall_;
	DuiLib::CButtonUI* btn_update_;

	Scheme* scheme_ = nullptr;

	std::wstring message_;
	std::wstring key_name_;
	std::wstring uninstall_path_;

	std::string local_version_;
	std::string last_version_;
	std::string download_url_;

	bool cracked_ = false;

	std::vector<std::map<std::wstring, std::wstring>> actions_;
};
