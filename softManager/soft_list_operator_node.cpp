#include "soft_list_operator_node.h"

#include "curl_download_manager.h"
#include "event_queue_global_manager.h"
#include "helper.h"
#include "kf_str.h"
#include "scheme.h"
#include "stringHelper.h"
#include "versionHelper.h"

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

void SoftListOperatorNode::UpdateMessage(const wchar_t* value) const {
	auto update_tab_layout = static_cast<DuiLib::CTabLayoutUI*>(GetItemAt(1));

	auto label_info = update_tab_layout->GetItemAt(2);

	label_info->SetText(value);
	label_info->SetToolTip(value);

	update_tab_layout->SelectItem(2);
}

void SoftListOperatorNode::UpdateUpgradeInfo(const char* last_version,
											 const char* download_url) {
	last_version_ = last_version;
	download_url_ = download_url;

	btn_update_->SetToolTip(CStringHelper::a2w(last_version).c_str());

	auto update_tab_layout = static_cast<DuiLib::CTabLayoutUI*>(GetItemAt(1));

	VersionHelper local_version(local_version_.c_str());
	VersionHelper remote_version(last_version_.c_str());

	if (remote_version > local_version) {
		// 显示更新按钮
		update_tab_layout->SelectItem(1);
	}
	else {
		// 该软件暂未加入更新计划
	}
}

void SoftListOperatorNode::Init() {
	DuiLib::CHorizontalLayoutUI::Init();

	m_pManager->AddNotifier(this);

	btn_uninstall_ = CreateUninstallButton();

	Add(btn_uninstall_);

	DuiLib::CTabLayoutUI* tab_layout = new DuiLib::CTabLayoutUI;

	// page 0: 暂未加入更新计划
	tab_layout->Add(new DuiLib::CControlUI);

	btn_update_ = CreateUpdateButton();

	// page 1: 更新提示
	tab_layout->Add(btn_update_);

	VersionHelper local_version(local_version_.c_str());
	VersionHelper remote_version(last_version_.c_str());

	if (remote_version > local_version) {
		// 显示更新按钮
		tab_layout->SelectItem(1);
	}
	else {
		// 该软件暂未加入更新计划
	}

	// page 2: 无需更新或者其他异常提醒
	tab_layout->Add(CreateInfoLabel());

	if (!message_.empty()) {
		// 无需更新之类的描述
		tab_layout->SelectItem(2);
	}

	Add(tab_layout);
}

void SoftListOperatorNode::NotifyClickedUpdateButton(DuiLib::TNotifyUI& msg) {
	if (CurlDownloadManager::InDownloadList(download_url_.c_str())) {
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

void SoftListOperatorNode::Notify(DuiLib::TNotifyUI& msg) {
	if (msg.sType == DUI_MSGTYPE_CLICK) {
		if (msg.pSender == btn_uninstall_) {
			Uninstall(uninstall_path_);
			return;
		}

		if (msg.pSender == btn_update_) {
			NotifyClickedUpdateButton(msg);
		}
	}
}

int SoftListOperatorNode::OnDownloadProgress(void* user_ptr, const char* url,
										   double total_download, double now_download, double speed) {
	auto pThis = static_cast<SoftListOperatorNode*>(user_ptr);

	if (pThis->download_url_ != url) {
		return 0;
	}

	if (now_download > 0.0) {
		auto buffer = KfString::Format("%0.2lf%%", now_download / total_download * 100.0f);
		pThis->btn_update_->SetText(buffer.GetWString().c_str());
		return 0;
	}

	pThis->btn_update_->SetText(L"0%");

	return 0;
}

void SoftListOperatorNode::OnDownloadFinished(void* user_ptr, const char* url, const char* file_path,
											  CURLcode code, int http_code) {
	auto pThis = static_cast<SoftListOperatorNode*>(user_ptr);
	if (pThis->download_url_ != url) {
		return;
	}

	if(CURLE_OK != code || 200 != http_code) {
		pThis->btn_update_->SetText((std::to_wstring(code) + std::to_wstring(http_code) + L"done.").c_str());
		return;
	}

	pThis->btn_update_->SetText(L"开始安装");

	auto path = CStringHelper::a2w(file_path);

	if (StrStrIA(file_path, ".exe") || StrStrIA(file_path, ".msi")) {
		Helper::ExecuteApplication(path.c_str(), L"");
		pThis->btn_update_->SetText(L"安装完成");
		EventQueueInstance->PostEvent(EVENT_UPDATE_SOFT_DATA,
									  reinterpret_cast<WPARAM>(pThis->key_name_.c_str()));
		return;
	}

	for (auto& it : pThis->actions_) {
		auto name = it[L"name"];

		if (name == L"clipboard") {
			Helper::UpdateClipboard(CStringHelper::w2a(it[L"text"]));
		}
	}

	Helper::OpenFolderAndSelectFile(path.c_str());

	pThis->btn_update_->SetText(L"下载完成");
}

void SoftListOperatorNode::OnDownloadNotify(void* user_ptr, const char* url, const char* msg) {
	auto pThis = static_cast<SoftListOperatorNode*>(user_ptr);
	if(pThis->download_url_ != url) {
		return;
	}

	auto buffer = CStringHelper::a2w(msg);
	pThis->btn_update_->SetText(buffer.c_str());
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
	auto btn_update = new DuiLib::CButtonUI;

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