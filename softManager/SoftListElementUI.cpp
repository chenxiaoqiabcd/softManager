#include "SoftListElementUI.h"

#include "event_queue_global_manager.h"
#include "file_helper.h"
#include "helper.h"
#include "scheme.h"
#include "soft_list_operator_node.h"
#include "stringHelper.h"

void CSoftListElementUI::SetScheme(Scheme* scheme) {
	scheme_ = scheme;
}

void CSoftListElementUI::SetSoftName(const wchar_t* value) {
	soft_name_ = value;
}

DuiLib::CDuiString CSoftListElementUI::GetSoftName() const {
	return soft_name_;
}

void CSoftListElementUI::SetIcon(const wchar_t* soft_icon, const wchar_t* key_name,
								 const wchar_t* install_location) {
	key_name_ = key_name;

	std::wstring icon;

	const auto extension = PathFindExtension(soft_icon);
	if (0 == wcscmp(extension, L".png")) {
		icon = soft_icon;
	}

	if (icon.empty() && key_name[0] == '{') {
		icon = FileHelper::GetIconWithGuid(key_name);
	}

	if (icon.empty() && 0 == _wcsicmp(extension, L".exe")) {
		icon = FileHelper::GetIconWithExePath(soft_icon);
	}

	if (icon.empty() && 0 == _wcsicmp(extension, L".ico")) {
		icon = FileHelper::GetIconWithIcoPath(soft_icon);
	}

	if(icon.empty()) {
		icon = FileHelper::GetIconWithExeOrIcoFile(install_location);
	}

	icon_ = icon.c_str();
}

void CSoftListElementUI::SetLocalVersion(const wchar_t* value) {
	local_version_ = value;
}

void CSoftListElementUI::SetBit(uint8_t bit) {
	bit_ = bit;
}

uint8_t CSoftListElementUI::GetBit() const {
	return bit_;
}

std::wstring CSoftListElementUI::GetDownloadUrl() const {
	return download_url_.GetData();
}

void CSoftListElementUI::SetUninstallPath(const wchar_t* value) {
	uninst_path_ = value;
}

void CSoftListElementUI::SetUpdateInfo(const std::string_view& last_version, 
									   const std::string_view& download_url,
									   const std::vector<std::map<std::wstring, std::wstring>>& actions,
									   bool cracked) {
	last_version_ = CStringHelper::a2w(last_version.data()).c_str();
	download_url_ = CStringHelper::a2w(download_url.data()).c_str();
	actions_ = actions;
	cracked_ = cracked;
}

void CSoftListElementUI::SetMessage(const wchar_t* value) {
	message_ = value;
}

void CSoftListElementUI::UpdateMessage(const wchar_t* value) const {
	operator_node_->UpdateMessage(value);
}

void CSoftListElementUI::UpdateUpgradeInfo(std::string_view last_version,
										   std::string_view download_url) const {
	operator_node_->UpdateUpgradeInfo(last_version.data(), download_url.data());
}

void CSoftListElementUI::UpdateSize(const wchar_t* value) const {
	GetItemAt(4)->SetText(value);
}

void CSoftListElementUI::InstallPackage(const wchar_t* file_path) {
	operator_node_->SetUpdateText(L"开始安装");

	if (StrStrIW(file_path, L".exe") || StrStrIW(file_path, L".msi")) {
		Helper::ExecuteApplication(file_path, L"");
		operator_node_->SetUpdateText(L"安装完成");
		EventQueueInstance->PostEvent(EVENT_UPDATE_SOFT_DATA,
									  reinterpret_cast<WPARAM>(key_name_.c_str()));

		return;
	}

	for (auto& it : actions_) {
		if (it[L"name"] == L"clipboard") {
			Helper::UpdateClipboard(CStringHelper::w2a(it[L"text"]));
		}
	}

	Helper::OpenFolderAndSelectFile(file_path);

	operator_node_->SetUpdateText(L"下载完成");
}

void CSoftListElementUI::DoInit() {
	SetFixedHeight(40);

	Add(CreateNumberNode());
	Add(CreateSoftNameNode());
	Add(CreateBitNode());
	Add(CreateLocalVersionNode());
	Add(CreatePackageSizeNode());
	Add(CreateOperatorNode());
}

DuiLib::CLabelUI* CSoftListElementUI::CreateNumberNode() {
	DuiLib::CLabelUI* label_number = new DuiLib::CLabelUI;

	label_number->SetAttribute(L"align", L"center");

	const auto& count = static_cast<DuiLib::CListUI*>(GetOwner())->GetCount();
	label_number->SetText(std::to_wstring(count + 1).c_str());

	scheme_->Refresh(label_number);

	return label_number;
}

DuiLib::CLabelUI* CSoftListElementUI::CreateSoftNameNode() const {
	DuiLib::CLabelUI* label_soft_name = new DuiLib::CLabelUI;

	DuiLib::CDuiString bk_image;
	bk_image.Format(TEXT("file=\'%s\' dest=\'4,4,36,36\'"), icon_);

	label_soft_name->SetBkImage(bk_image);
	label_soft_name->SetTextPadding({ 48, 0, 0, 0 });
	label_soft_name->SetText(soft_name_);
	label_soft_name->SetToolTip(soft_name_);
	label_soft_name->SetAttribute(L"endellipsis", L"true");

	scheme_->Refresh(label_soft_name);

	return label_soft_name;
}

DuiLib::CLabelUI* CSoftListElementUI::CreateBitNode() const {
	DuiLib::CLabelUI* label_bit = new DuiLib::CLabelUI;

	label_bit->SetAttribute(L"align", L"center");

	label_bit->SetText(std::to_wstring(bit_).c_str());

	scheme_->Refresh(label_bit);

	return label_bit;
}

DuiLib::CLabelUI* CSoftListElementUI::CreateLocalVersionNode() const {
	DuiLib::CLabelUI* label_local_version = new DuiLib::CLabelUI;

	label_local_version->SetTextPadding({ 5,0,0,0 });

	label_local_version->SetText(local_version_);

	scheme_->Refresh(label_local_version);

	return label_local_version;
}

DuiLib::CLabelUI* CSoftListElementUI::CreatePackageSizeNode() const {
	DuiLib::CLabelUI* label_size = new DuiLib::CLabelUI;
	scheme_->Refresh(label_size);
	return label_size;
}

DuiLib::CControlUI* CSoftListElementUI::CreateOperatorNode() {
	operator_node_ = new SoftListOperatorNode;

	operator_node_->SetScheme(scheme_);
	operator_node_->SetMessage(message_);
	operator_node_->SetKeyName(key_name_.c_str());
	operator_node_->SetUninstallPath(uninst_path_);
	operator_node_->SetLocalVersion(CStringHelper::w2a(local_version_.GetData()).c_str());
	operator_node_->SetUpdateInfo(CStringHelper::w2a(last_version_.GetData()).c_str(),
								  CStringHelper::w2a(download_url_.GetData()).c_str(), actions_, cracked_);

	return operator_node_;
}











void CUpdateListElementUI::DoInit() {
	SetFixedHeight(40);

	Add(CreateNumberNode());
	Add(CreateSoftNameNode());
	Add(CreateBitNode());
	Add(CreateLocalVersionNode());
	Add(CreateLastVersionNode());
	Add(CreateOperatorNode());
}

DuiLib::CLabelUI* CUpdateListElementUI::CreateLastVersionNode() const {
	DuiLib::CLabelUI* label_local_version = new DuiLib::CLabelUI;

	label_local_version->SetTextPadding({ 5,0,0,0 });

	label_local_version->SetText(last_version_);
	scheme_->Refresh(label_local_version);

	return label_local_version;
}