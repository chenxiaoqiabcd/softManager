#pragma once

#include "WndImpl.h"
#include "observerMode.h"

class CUpdateListElementUI;
struct UpdateInfo;
class KfString;
struct SoftInfo;

class CUpdateWnd : public CWndImpl, public IDisplay
{
public:
	~CUpdateWnd();
protected:
	LPCTSTR GetSkinFile() override;

	void Init() override;

	void Notify(DuiLib::TNotifyUI& msg) override;

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	CUpdateListElementUI* CreateLine(const SoftInfo& soft_it, const UpdateInfo* info);

	void UpdateDate(bool need_update, void* data, size_t update_count) override;

	void ClearData() override;

	void ClearData(void* data) override;

	void UpdateDate();

	void RemoveLine(const wchar_t* soft_name) const;

	static DWORD OnRefreshUpdateWndList(WPARAM wParam, LPARAM lParam, LPVOID data);

	static DWORD OnInstallPackage(WPARAM wParam, LPARAM lParam, LPVOID user_ptr);
private:
	std::vector<std::tuple<KfString, uint8_t>> update_info_vec_;
};
