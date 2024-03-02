#include "SoftListElementUI.h"

#include "event_queue_global_manager.h"
#include "file_helper.h"
#include "helper.h"
#include "installPackageWnd.h"
#include "kf_log.h"
#include "scheme.h"
#include "stringHelper.h"
#include "versionHelper.h"

CSoftListElementUI::~CSoftListElementUI() {
	m_pManager->RemoveNotifier(this);
}

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
	const auto hor = static_cast<DuiLib::CHorizontalLayoutUI*>(GetItemAt(5));
	const auto tab_layout = static_cast<DuiLib::CTabLayoutUI*>(hor->GetItemAt(1));

	tab_layout->GetItemAt(2)->SetText(value);
	tab_layout->GetItemAt(2)->SetToolTip(value);

	tab_layout->SelectItem(2);
}

void CSoftListElementUI::UpdateUpgradeInfo(std::string_view last_version,
										   std::string_view download_url) const {
	const auto hor = static_cast<DuiLib::CHorizontalLayoutUI*>(GetItemAt(5));
	const auto tab_layout = static_cast<DuiLib::CTabLayoutUI*>(hor->GetItemAt(1));

	const auto btn_update = static_cast<DuiLib::CButtonUI*>(tab_layout->GetItemAt(1));
	if(nullptr == btn_update) {
		return;
	}

	btn_update->SetUserData(CStringHelper::a2w(download_url.data()).c_str());
	btn_update->SetToolTip(CStringHelper::a2w(last_version.data()).c_str());

	tab_layout->SelectItem(1);
}

void CSoftListElementUI::UpdateSize(const wchar_t* value) const {
	GetItemAt(4)->SetText(value);
}

void CSoftListElementUI::DoInit() {
	m_pManager->AddNotifier(this);

	SetFixedHeight(40);

	Add(CreateNumberNode());
	Add(CreateSoftNameNode());
	Add(CreateBitNode());
	Add(CreateLocalVersionNode());
	Add(CreatePackageSizeNode());
	Add(CreateOperatorNode());
}

void CSoftListElementUI::NotifyClickedUpdateButton(DuiLib::TNotifyUI& msg) const {
	CInstallPackageWnd wnd;
	wnd.SetInfo(download_url_, actions_);
	wnd.Create(m_pManager->GetPaintWindow(), L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	wnd.CenterWindow();
	if (IDOK == wnd.ShowModal()) {
		EventQueueInstance->PostEvent(EVENT_UPDATE_SOFT_DATA, reinterpret_cast<WPARAM>(key_name_.c_str()));
	}
}

void CSoftListElementUI::Notify(DuiLib::TNotifyUI& msg) {
	if(msg.sType == DUI_MSGTYPE_CLICK) {
		if(msg.pSender == btn_uninst_) {
			Uninstall(uninst_path_.GetData());
			return;
		}

		if(msg.pSender == btn_update_) {
			NotifyClickedUpdateButton(msg);
		}
	}
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

DuiLib::CLabelUI* CSoftListElementUI::CreatePackageSizeNode() {
	DuiLib::CLabelUI* label_size = new DuiLib::CLabelUI;
	scheme_->Refresh(label_size);
	return label_size;
}

DuiLib::CHorizontalLayoutUI* CSoftListElementUI::CreateOperatorNode() {
	DuiLib::CHorizontalLayoutUI* hor = new DuiLib::CHorizontalLayoutUI;

	btn_uninst_ = new DuiLib::CButtonUI;

	btn_uninst_->SetText(TEXT("卸载"));
	btn_uninst_->SetFixedWidth(60);
	btn_uninst_->SetFixedHeight(32);
	btn_uninst_->SetPadding({ 4, 4, 0, 0 });
	btn_uninst_->SetBorderColor(0xFF999999);
	btn_uninst_->SetBorderSize(1);
	scheme_->Refresh(btn_uninst_);

	hor->Add(btn_uninst_);

	DuiLib::CTabLayoutUI* tab_layout = new DuiLib::CTabLayoutUI;

	btn_update_ = new DuiLib::CButtonUI;

	tab_layout->Add(new DuiLib::CControlUI);

	btn_update_->SetName(L"update_btn");

	if(cracked_) {
		btn_update_->SetText(TEXT("更新(破解版)"));
	}
	else {
		btn_update_->SetText(TEXT("更新"));
	}

	btn_update_->SetPadding({ 4, 4, 4, 4 });
	btn_update_->SetToolTip(last_version_);
	scheme_->Refresh(btn_update_);

	tab_layout->Add(btn_update_);

	DuiLib::CLabelUI* label_info = new DuiLib::CLabelUI;

	label_info->SetAttribute(L"endellipsis", L"true");
	label_info->SetTextPadding({ 5,0,0,0 });
	label_info->SetText(message_);
	label_info->SetToolTip(message_);
	scheme_->Refresh(label_info);

	tab_layout->Add(label_info);

	if (!message_.IsEmpty()) {
		tab_layout->SelectItem(2);
	}

	VersionHelper local_version(CStringHelper::w2a(local_version_.GetData()).c_str());
	VersionHelper remote_version(CStringHelper::w2a(last_version_.GetData()).c_str());

	if (remote_version > local_version) {
		tab_layout->SelectItem(1);
	}

	hor->Add(tab_layout);

	return hor;
}

void CSoftListElementUI::Uninstall(std::wstring_view cmd) {
	if(cmd.empty()) {
		return;
	}

	const auto cmd_len = cmd.length();

	if (0 == wcscmp(cmd.substr(0, 1).data(), L"\"") &&
		0 == wcscmp(cmd.substr(cmd_len - 1, 0).data(), L"\"")) {
		cmd = cmd.substr(1, cmd_len - 2);
	}

	if(PathFileExists(cmd.data())) {
		Helper::ExecuteApplication(cmd.data(), L"");
	}
	else {
		int nArgs = 0;
		const LPWSTR* lpszCmdLine = CommandLineToArgvW(cmd.data(), &nArgs);
		if (1 == nArgs) {
			Helper::ExecuteApplication(lpszCmdLine[0], L"");
		}
		else if (2 == nArgs) {
			Helper::ExecuteApplication(lpszCmdLine[0], lpszCmdLine[1]);
		}
		else {
			assert(false && L"卸载程序执行多个参数这个情况暂未考虑");
		}
	}

	EventQueueInstance->PostEvent(EVENT_UPDATE_SOFT_DATA, reinterpret_cast<WPARAM>(key_name_.c_str()));
}











void CUpdateListElementUI::DoInit() {
	m_pManager->AddNotifier(this);

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