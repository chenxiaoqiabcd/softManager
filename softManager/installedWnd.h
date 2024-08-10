#pragma once

#include <thread>

#include "observerMode.h"
#include "WndImpl.h"

class CSoftListElementUI;
struct SoftInfo;

class CInstalledWnd : public CWndImpl, public IDisplay
{
public:
	virtual ~CInstalledWnd();

	void UpdateSoftInfo();
protected:
	LPCTSTR GetSkinFile() override;

	void Init() override;

	void Notify(DuiLib::TNotifyUI& msg) override;

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	static DWORD WINAPI ThreadUpdateSoftListV2(LPVOID lParam);

	void UpdateDate(bool need_update, void* data) override;

	void ClearData() override;

	void ClearData(void* data) override;

	static DWORD OnInstallPackage(WPARAM wParam, LPARAM lParam, LPVOID user_ptr);
private:
	LONGLONG GetInstallPathSize(const wchar_t* install_path, const wchar_t* uninstall_path);

	void UpdateSize(const SoftInfo& info);

	CSoftListElementUI* CreateLine(const SoftInfo& soft_info) const;

	DuiLib::CDuiString find_text_;

	std::vector<SoftInfo> soft_size_list_;

	std::thread update_soft_size_thread_;

	bool stop_update_soft_list_ = false;
};
