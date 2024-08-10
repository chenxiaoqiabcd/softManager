#include "updateWnd.h"

#include <algorithm>

#include "curl_download_manager.h"
#include "event_queue_global_manager.h"
#include "log_helper.h"
#include "kf_str.h"
#include "softInfo.h"
#include "SoftListElementUI.h"
#include "stringHelper.h"
#include "UpdateDetection.h"
#include "versionHelper.h"

CUpdateWnd::~CUpdateWnd() {
	UpdateInstance->GetDataCenter()->Detach(this);
}

LPCTSTR CUpdateWnd::GetSkinFile() {
	return L"updateWnd.xml";
}

void CUpdateWnd::Init() {
	EventQueueInstance->AppendNewThreadListener(EVENT_REFRESH_UPDATE_WND_SOFT_LIST, OnRefreshUpdateWndList, this);

	EventQueueInstance->AppendNewThreadListener(EVENT_INSTALL_PACKAGE, OnInstallPackage, this);

	UpdateInstance->GetDataCenter()->Attach(this);
}

void CUpdateWnd::Notify(DuiLib::TNotifyUI& msg) {
	if(msg.sType == DUI_MSGTYPE_CLICK) {
		DuiLib::CDuiString name = msg.pSender->GetName();

		if(0 == _wcsicmp(name, L"closebtn")) {
			Close(0);
		}
	}
}

LRESULT CUpdateWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if(uMsg == WM_CLOSE) {
		CurlDownloadManager::Quit();
	}

	return CWndImpl::HandleMessage(uMsg, wParam, lParam);
}

CUpdateListElementUI* CUpdateWnd::CreateLine(const SoftInfo& soft_it, const UpdateInfo* info) {
	VersionHelper local_version(soft_it.m_strSoftVersion.GetString());
	VersionHelper remote_version(info->version.c_str());

	if(local_version < remote_version) {
		auto find_update_info = [&soft_it](std::tuple<KfString, uint8_t>& data) {
			return std::get<0>(data) == soft_it.m_strSoftName.GetString()
				&& std::get<1>(data) == soft_it.bit;
			};

		if (std::any_of(update_info_vec_.begin(), update_info_vec_.end(), find_update_info)) {
			return nullptr;
		}

		update_info_vec_.emplace_back(soft_it.m_strSoftName.GetString(), soft_it.bit);

		auto line = new CUpdateListElementUI;

		line->SetScheme(scheme_.get());
		line->SetSoftName(soft_it.m_strSoftName);
		line->SetIcon(soft_it.m_strSoftIcon,
					  soft_it.key_name,
					  soft_it.m_strInstallLocation);
		line->SetBit(soft_it.bit);
		line->SetLocalVersion(soft_it.m_strSoftVersion);
		line->SetUninstallPath(soft_it.m_strUninstallPth);
		line->SetUpdateInfo(info->version, info->url, info->actions,
							info->type == L"cracked");
		line->SetFixedHeight(40);

		return line;
	}

	return nullptr;
}

void CUpdateWnd::UpdateDate(bool need_update, void* data) {
	const auto info = static_cast<UpdateInfo*>(data);

	if(!need_update) {
		return;
	}

	const auto pList = dynamic_cast<DuiLib::CListUI*>(m_pm.FindControl(L"soft_list"));

	for (const auto& soft_it : CSoftInfo::GetInstance()->GetSoftInfo()) {
		if(!CStringHelper::IsMatch(soft_it.m_strSoftName.GetString(), info->name.c_str())) {
			continue;
		}

		auto line = CreateLine(soft_it, info);
		if (nullptr == line) {
			continue;
		}

		pList->Add(line);
	}
}

void CUpdateWnd::ClearData() {
	update_info_vec_.clear();

	const auto pList = dynamic_cast<DuiLib::CListUI*>(m_pm.FindControl(L"soft_list"));
	pList->RemoveAll();
}

void CUpdateWnd::ClearData(void* data) {
	const auto info = static_cast<UpdateInfo*>(data);
	RemoveLine(CStringHelper::a2w(info->name).c_str());
}

void CUpdateWnd::UpdateDate() {
	const auto pList = dynamic_cast<DuiLib::CListUI*>(m_pm.FindControl(L"soft_list"));

	for(const auto& soft_it : CSoftInfo::GetInstance()->GetSoftInfo()) {
		auto match_result = UpdateInstance->MatchName(soft_it.m_strSoftName.GetString());
		if(!match_result.has_value()) {
			continue;
		}

		const auto line = CreateLine(soft_it, &match_result.value());
		if (nullptr == line) {
			continue;
		}

		pList->Add(line);
	}
}

void CUpdateWnd::RemoveLine(const wchar_t* soft_name) const {
	const auto pList = dynamic_cast<DuiLib::CListUI*>(m_pm.FindControl(L"soft_list"));

	for (int i = 0; i < pList->GetCount(); ++i) {
		const auto line = dynamic_cast<CUpdateListElementUI*>(pList->GetItemAt(i));
		if (nullptr == line) {
			continue;
		}

		if (0 == _wcsicmp(line->GetSoftName(), soft_name)) {
			pList->RemoveAt(i);
			break;
		}
	}
}

DWORD CUpdateWnd::OnRefreshUpdateWndList(WPARAM wParam, LPARAM lParam, LPVOID data) {
	auto pThis = static_cast<CUpdateWnd*>(data);

	if (!IsWindow(pThis->m_hWnd)) {
		return 0;
	}

	const auto soft_list = static_cast<DuiLib::CListUI*>(pThis->m_pm.FindControl(L"soft_list"));

	if (nullptr != soft_list) {
		pThis->ClearData();
		pThis->UpdateDate();
		return 0;
	}

	KF_WARN("soft_list没找到，可以考虑再添加一点延迟");
	return 0;
}

DWORD CUpdateWnd::OnInstallPackage(WPARAM wParam, LPARAM lParam, LPVOID user_ptr) {
	auto pThis = static_cast<CUpdateWnd*>(user_ptr);

	auto file_path = reinterpret_cast<const char*>(wParam);
	auto url = CStringHelper::a2w(reinterpret_cast<const char*>(lParam));

	const auto pList = dynamic_cast<DuiLib::CListUI*>(pThis->m_pm.FindControl(L"soft_list"));

	auto count = pList->GetCount();

	for(int index = 0; index < count; ++index) {
		auto line = dynamic_cast<CUpdateListElementUI*>(pList->GetItemAt(index));

		if(line->GetDownloadUrl() == url) {
			line->InstallPackage(CStringHelper::a2w(file_path).c_str());
			break;
		}
	}

	return 0;
}
