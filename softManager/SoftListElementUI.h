#pragma once

#include <string_view>

#include <UIlib.h>

class SoftListOperatorNode;
class CurlDownloadManager;
class Scheme;

class CSoftListElementUI : public DuiLib::CListContainerElementUI {
public:
	void SetScheme(Scheme* scheme);

	void SetSoftName(const wchar_t* value);

	DuiLib::CDuiString GetSoftName() const;

	void SetIcon(const wchar_t* soft_icon, const wchar_t* key_name, const wchar_t* install_location);

	void SetLocalVersion(const wchar_t* value);

	void SetBit(uint8_t bit);

	uint8_t GetBit() const;

	std::wstring GetDownloadUrl() const;

	void SetUninstallPath(const wchar_t* value);

	void SetUpdateInfo(const std::string_view& last_version, const std::string_view& download_url,
					   const std::vector<std::map<std::wstring, std::wstring>>& actions, bool cracked);

	void SetMessage(const wchar_t* value);

	void UpdateMessage(const wchar_t* value) const;

	void UpdateUpgradeInfo(std::string_view last_version, std::string_view download_url) const;

	void UpdateSize(const wchar_t* value) const;

	void InstallPackage(const wchar_t* file_path);
protected:
	void DoInit() override;

	// 创建序号节点
	DuiLib::CLabelUI* CreateNumberNode();

	DuiLib::CLabelUI* CreateSoftNameNode() const;

	DuiLib::CLabelUI* CreateBitNode() const;							// 创建位数节点

	DuiLib::CLabelUI* CreateLocalVersionNode() const;					// 创建本机版本节点

	DuiLib::CLabelUI* CreatePackageSizeNode() const;					// 创建软件大小节点

	DuiLib::CControlUI* CreateOperatorNode();

	DuiLib::CDuiString last_version_;

	Scheme* scheme_ = nullptr;
private:
	SoftListOperatorNode* operator_node_ = nullptr;

	std::vector<std::map<std::wstring, std::wstring>> actions_;

	std::wstring key_name_;

	DuiLib::CDuiString soft_name_;

	DuiLib::CDuiString icon_;

	DuiLib::CDuiString local_version_;

	DuiLib::CDuiString download_url_;

	DuiLib::CDuiString message_;

	DuiLib::CDuiString uninst_path_;

	bool cracked_ = false;

	uint8_t bit_ = 0;
};











class CUpdateListElementUI : public CSoftListElementUI {
protected:
	void DoInit() override;

	DuiLib::CLabelUI* CreateLastVersionNode() const;		// 创建最新版本节点
};