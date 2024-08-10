#include "soft_list_operator_node.h"

#include "curl_download_manager.h"
#include "event_queue_global_manager.h"
#include "helper.h"
#include "kf_str.h"
#include "scheme.h"
#include "stringHelper.h"
#include "versionHelper.h"

void UpdateButtonUI::SetDownloadUrl(const char* url) {
	download_url_ = url;
}

void UpdateButtonUI::SetKeyName(const wchar_t* value) {
	key_name_ = value;
}

void UpdateButtonUI::SetActions(const std::vector<std::map<std::wstring, std::wstring>>& actions) {
	actions_ = actions;
}

void UpdateButtonUI::DoInit() {
	DuiLib::CButtonUI::DoInit();

	m_pManager->AddNotifier(this);
}

void UpdateButtonUI::NotifyClickedUpdateButton(DuiLib::TNotifyUI& msg) {
	if (CurlDownloadManager::InDownloadList(download_url_.c_str())) {
		// 如果暂停了就继续下载，如果没暂停就暂停下载
		if (CurlDownloadManager::IsPaused(download_url_.c_str())) {
			CurlDownloadManager::ResumeTask(download_url_.c_str());
		}
		else {
			CurlDownloadManager::PauseTask(download_url_.c_str());
		}

		return;
	}

	CurlDownloadManager download_manager;
	download_manager.AddTask(download_url_.c_str(),
							 OnDownloadProgress, OnDownloadFinished, OnDownloadNotify, this);
}

void UpdateButtonUI::Notify(DuiLib::TNotifyUI& msg) {
	if(msg.pSender == this && msg.sType == DUI_MSGTYPE_CLICK) {
		NotifyClickedUpdateButton(msg);
	}
}

int UpdateButtonUI::OnDownloadProgress(void* user_ptr, const char* url,
											 double total_download, double now_download, double speed) {
	auto pThis = static_cast<UpdateButtonUI*>(user_ptr);

	if (pThis->download_url_ != url) {
		return 0;
	}

	if (now_download > 0.0) {
		auto buffer = KfString::Format("%0.2lf%%", now_download / total_download * 100.0f);
		pThis->SetText(buffer.GetWString().c_str());
		return 0;
	}

	pThis->SetText(L"0%");

	return 0;
}

void UpdateButtonUI::OnDownloadFinished(void* user_ptr, const char* url, const char* file_path,
									  CURLcode code, int http_code) {
	auto pThis = static_cast<UpdateButtonUI*>(user_ptr);
	if (pThis->download_url_ != url) {
		return;
	}

	if(200 != http_code) {
		// TODO: 发送请求给管理员更新下载链接
	}

	if(CURLE_OK != code || 200 != http_code) {
		auto buffer = KfString::Format(L"curl code: %d, http code: %d", code, http_code);
		pThis->SetText(L"下载失败");
		pThis->SetToolTip(buffer.GetWString().c_str());
		return;
	}

	EventQueueInstance->PostEvent(EVENT_INSTALL_PACKAGE, reinterpret_cast<WPARAM>(file_path), reinterpret_cast<LPARAM>(url));
}

void UpdateButtonUI::OnDownloadNotify(void* user_ptr, const char* url, const char* msg) {
	auto pThis = static_cast<UpdateButtonUI*>(user_ptr);
	if (pThis->download_url_ != url) {
		return;
	}

	auto buffer = CStringHelper::a2w(msg);
	pThis->SetText(buffer.c_str());
}











SoftListOperatorNode::~SoftListOperatorNode() {
	m_pManager->RemoveNotifier(this);
}

void SoftListOperatorNode::SetScheme(Scheme* scheme) {
	scheme_ = scheme;
}

void SoftListOperatorNode::SetMessage(const wchar_t* value) {
	message_ = value;
}

void SoftListOperatorNode::SetKeyName(const wchar_t* value) {
	key_name_ = value;
}

void SoftListOperatorNode::SetUninstallPath(const wchar_t* value) {
	uninstall_path_ = value;
}

void SoftListOperatorNode::SetLocalVersion(const char* value) {
	local_version_ = value;
}

void SoftListOperatorNode::SetUpdateInfo(const char* last_version, const char* download_url,
										 const std::vector<std::map<std::wstring, std::wstring>>& actions,
										 bool cracked) {
	last_version_ = last_version;
	download_url_ = download_url;
	actions_ = actions;
	cracked_ = cracked;	
}

void SoftListOperatorNode::SetUpdateText(const wchar_t* value) const {
	btn_update_->SetText(value);
}

void SoftListOperatorNode::UpdateMessage(const wchar_t* value) {
	message_ = value;
	auto update_tab_layout = static_cast<DuiLib::CTabLayoutUI*>(GetItemAt(1));

	auto label_info = update_tab_layout->GetItemAt(2);

	label_info->SetText(value);
	label_info->SetToolTip(value);

	UpdateState();
}

void SoftListOperatorNode::UpdateUpgradeInfo(const char* last_version,
											 const char* download_url) {
	last_version_ = last_version;
	download_url_ = download_url;

	btn_update_->SetToolTip(CStringHelper::a2w(last_version).c_str());

	UpdateState();
}

void SoftListOperatorNode::Init() {
	DuiLib::CHorizontalLayoutUI::Init();

	m_pManager->AddNotifier(this);

	btn_uninstall_ = CreateUninstallButton();

	Add(btn_uninstall_);

	tab_layout_ = new DuiLib::CTabLayoutUI;

	// page 0: 暂未加入更新计划
	tab_layout_->Add(new DuiLib::CControlUI);

	btn_update_ = CreateUpdateButton();

	// page 1: 更新提示
	tab_layout_->Add(btn_update_);

	// page 2: 无需更新或者其他异常提醒
	tab_layout_->Add(CreateInfoLabel());

	Add(tab_layout_);

	UpdateState();
}

void SoftListOperatorNode::Notify(DuiLib::TNotifyUI& msg) {
	if (msg.sType == DUI_MSGTYPE_CLICK) {
		if (msg.pSender == btn_uninstall_) {
			Uninstall(uninstall_path_);
		}
	}
}

DuiLib::CButtonUI* SoftListOperatorNode::CreateUninstallButton() const {
	auto btn_uninstall = new DuiLib::CButtonUI;

	btn_uninstall->SetText(TEXT("卸载"));
	btn_uninstall->SetFixedWidth(60);
	btn_uninstall->SetFixedHeight(32);
	btn_uninstall->SetPadding({ 4, 4, 0, 0 });
	btn_uninstall->SetBorderColor(0xFF999999);
	btn_uninstall->SetBorderSize(1);
	scheme_->Refresh(btn_uninstall);

	return btn_uninstall;
}

DuiLib::CButtonUI* SoftListOperatorNode::CreateUpdateButton() const {
	auto btn_update = new UpdateButtonUI;

	btn_update->SetDownloadUrl(download_url_.c_str());
	btn_update->SetKeyName(key_name_.c_str());
	btn_update->SetActions(actions_);

	btn_update->SetName(L"update_btn");

	if (cracked_) {
		btn_update->SetText(TEXT("更新(破解版)"));
	}
	else {
		btn_update->SetText(TEXT("更新"));
	}

	btn_update->SetPadding({ 4, 4, 4, 4 });
	btn_update->SetToolTip(CStringHelper::a2w(last_version_).c_str());
	scheme_->Refresh(btn_update);

	return btn_update;
}

DuiLib::CLabelUI* SoftListOperatorNode::CreateInfoLabel() const {
	DuiLib::CLabelUI* label_info = new DuiLib::CLabelUI;

	label_info->SetAttribute(L"endellipsis", L"true");
	label_info->SetTextPadding({ 5,0,0,0 });
	label_info->SetText(message_.c_str());
	label_info->SetToolTip(message_.c_str());
	scheme_->Refresh(label_info);

	return label_info;
}

void SoftListOperatorNode::Uninstall(std::wstring_view cmd) {
	if (cmd.empty()) {
		return;
	}

	const auto cmd_len = cmd.length();

	if (0 == wcscmp(cmd.substr(0, 1).data(), L"\"") &&
		0 == wcscmp(cmd.substr(cmd_len - 1, 0).data(), L"\"")) {
		cmd = cmd.substr(1, cmd_len - 2);
	}

	if (PathFileExists(cmd.data())) {
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

void SoftListOperatorNode::UpdateState() const {
	auto count = tab_layout_->GetCount();

	VersionHelper local_version(local_version_.c_str());
	VersionHelper remote_version(last_version_.c_str());

	if (remote_version > local_version) {
		// 显示更新按钮
		tab_layout_->SelectItem(1);
	}
	else {
		// 该软件暂未加入更新计划
	}

	if (!message_.empty()) {
		// 无需更新之类的描述
		tab_layout_->SelectItem(2);
	}
}
