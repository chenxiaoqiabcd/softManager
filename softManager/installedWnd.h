#pragma once

#include <mutex>

#include "observerMode.h"
#include "WndImpl.h"

class CSoftListElementUI;
struct SoftInfo;

class CInstalledWnd : public CWndImpl, public IDisplay
{
public:
	~CInstalledWnd();

	void UpdateSoftInfo();
protected:
	LPCTSTR GetSkinFile() override;
	void Init() override;

	void Notify(DuiLib::TNotifyUI& msg) override;

	static DWORD WINAPI ThreadUpdateSoftListV2(LPVOID lParam);

	static DWORD WINAPI ThreadUpdateSoftSize(LPVOID lParam);

	void UpdateDate(bool need_update, void* data) override;

	void ClearData() override;
private:
	LONGLONG GetInstallPathSize(const wchar_t* install_path, const wchar_t* uninstall_path);

	void UpdateSize(const wchar_t* soft_name, uint8_t bit, const wchar_t* install_path,
					const wchar_t* uninstall_path);

	CSoftListElementUI* CreateLine(const SoftInfo& soft_info) const;

	DuiLib::CDuiString find_text_;

	// std::map<CString, std::tuple<CString, CString>> soft_size_map_;

	// soft name, bit, install location, uninstall path
	std::vector<std::tuple<CString, uint8_t, CString, CString>> soft_size_list_;
};
