#pragma once

#include <string_view>

#include <UIlib.h>

class Scheme;

class CSoftListElementUI : public DuiLib::CListContainerElementUI, public DuiLib::INotifyUI {
public:
	~CSoftListElementUI();

	void SetScheme(Scheme* scheme);

	void SetSoftName(const wchar_t* value);

	DuiLib::CDuiString GetSoftName() const;

	void SetIcon(const wchar_t* soft_icon, const wchar_t* key_name, const wchar_t* install_location);

	void SetLocalVersion(const wchar_t* value);

	void SetBit(uint8_t bit);

	uint8_t GetBit() const;

	void SetUninstallPath(const wchar_t* value);

	void SetUpdateInfo(const std::string_view& last_version, const std::string_view& download_url,
					   const std::vector<std::map<std::wstring, std::wstring>>& actions, bool cracked);

	void SetMessage(const wchar_t* value);

	void UpdateMessage(const wchar_t* value) const;

	void UpdateUpgradeInfo(std::string_view last_version, std::string_view download_url) const;

	void UpdateSize(const wchar_t* value) const;
protected:
	void DoInit() override;

	void NotifyClickedUpdateButton(DuiLib::TNotifyUI& msg) const;

	void Notify(DuiLib::TNotifyUI& msg) override;

	static DWORD WINAPI ThreadUpdateSize(LPVOID lParam);

	// 创建序号节点
	DuiLib::CLabelUI* CreateNumberNode();

	DuiLib::CLabelUI* CreateSoftNameNode() const;

	DuiLib::CLabelUI* CreateBitNode() const;							// 创建位数节点

	DuiLib::CLabelUI* CreateLocalVersionNode() const;					// 创建本机版本节点

	DuiLib::CLabelUI* CreatePackageSizeNode();					// 创建软件大小节点

	DuiLib::CHorizontalLayoutUI* CreateOperatorNode();			// 创建操作节点

	void Uninstall(std::wstring_view cmd);

	DuiLib::CDuiString last_version_;

	Scheme* scheme_ = nullptr;
private:
	DuiLib::CDuiString soft_name_;

	DuiLib::CDuiString icon_;

	DuiLib::CDuiString local_version_;

	DuiLib::CDuiString download_url_;

	DuiLib::CDuiString message_;

	DuiLib::CDuiString uninst_path_;

	DuiLib::CButtonUI* btn_uninst_;

	DuiLib::CButtonUI* btn_update_;

	std::vector<std::map<std::wstring, std::wstring>> actions_;

	std::wstring key_name_;

	bool cracked_ = false;

	uint8_t bit_ = 0;
};











class CUpdateListElementUI : public CSoftListElementUI {
protected:
	void DoInit() override;

	DuiLib::CLabelUI* CreateLastVersionNode() const;		// 创建最新版本节点
};