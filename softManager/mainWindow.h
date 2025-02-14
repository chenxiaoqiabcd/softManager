#pragma once

#include <memory>
#include <string_view>

#include <curl/curl.h>

#include "WndImpl.h"
#include "observerMode.h"

class CInstalledWnd;
class CUpdateWnd;
class UpdateMainWindow;

class CMainWindow : public CWndImpl, public IDisplay
{
public:
	~CMainWindow();

	void NotifyClickMenu(DuiLib::TNotifyUI& msg);

	void Notify(DuiLib::TNotifyUI& msg) override;
protected:
	LPCTSTR GetSkinFile() override;

	void Init() override;

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	static DWORD WINAPI ThreadGetUpdateInfo(LPVOID lParam);

	void UpdateDate(bool need_update, void* data,
					size_t update_count) override;

	void UpdateUpdateCount(int value) const;

	void ClearData() override;

	void ClearData(void* data) override;

	static DWORD OnEventUpdateStatusLabel(WPARAM wParam, LPARAM lParam, LPVOID data);

	static DWORD OnEventUpdateSoftList(WPARAM wParam, LPARAM lParam, LPVOID data);

	static DWORD OnEventUpdateSoftData(WPARAM wParam, LPARAM lParam, LPVOID data);

	static void DownloadFinishedCallback(CURLcode code, void* ptr, std::wstring_view file_path);
private:
	bool CheckUpdate();

	std::shared_ptr<CInstalledWnd> installed_wnd_;
	std::shared_ptr<CUpdateWnd> update_wnd_;
	std::shared_ptr<UpdateMainWindow> update_main_wnd_;

	DuiLib::CMenuWnd* menu_wnd_ = nullptr;
};
