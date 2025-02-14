#include "installedWnd.h"

#include "curl_download_manager.h"
#include "event_queue_global_manager.h"
#include "file_helper.h"
#include "helper.h"
#include "letter_helper.h"
#include "softInfo.h"
#include "SoftListElementUI.h"
#include "stringHelper.h"
#include "UpdateDetection.h"

CInstalledWnd::~CInstalledWnd() {
	EventQueueInstance->RemoveNewThreadListener(this);
	UpdateInstance->GetDataCenter()->Detach(this);
}

void CInstalledWnd::UpdateSoftInfo() {
	const HANDLE hThread = CreateThread(nullptr, 0, ThreadUpdateSoftListV2, this, 0, nullptr);
	CloseHandle(hThread);
}

LPCTSTR CInstalledWnd::GetSkinFile() {
	return L"installedWnd.xml";
}

void CInstalledWnd::Init() {
	UpdateInstance->GetDataCenter()->Attach(this);

	EventQueueInstance->AppendNewThreadListener(EVENT_INSTALL_PACKAGE, OnInstallPackage, this);

	const HANDLE hThread = CreateThread(nullptr, 0, ThreadUpdateSoftListV2, this, 0, nullptr);
	CloseHandle(hThread);
}

void CInstalledWnd::Notify(DuiLib::TNotifyUI& msg) {
	if(msg.sType == DUI_MSGTYPE_CLICK) {
		wstring strName = msg.pSender->GetName().GetData();
		if (0 == _wcsicmp(strName.c_str(), L"closebtn")) {
			Close(0);
			return;
		}

		if(0 == _wcsicmp(strName.c_str(), L"search_btn")) {
			find_text_ = m_pm.FindControl(L"search_edit")->GetText();

			stop_update_soft_list_ = true;

			if (update_soft_size_thread_.joinable()) {
				update_soft_size_thread_.join();
			}

			HANDLE hThread = CreateThread(nullptr, 0, ThreadUpdateSoftListV2, this, 0, nullptr);
			CloseHandle(hThread);
		}
	}
}

LRESULT CInstalledWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if(WM_CLOSE == uMsg) {
		stop_update_soft_list_ = true;

		CurlDownloadManager::Quit();

		if(update_soft_size_thread_.joinable()) {
			update_soft_size_thread_.join();
		}
	}

	return CWndImpl::HandleMessage(uMsg, wParam, lParam);
}

DWORD CInstalledWnd::ThreadUpdateSoftListV2(LPVOID lParam) {
	CInstalledWnd* pThis = static_cast<CInstalledWnd*>(lParam);

	DuiLib::CListUI* pList = static_cast<DuiLib::CListUI*>(pThis->m_pm.FindControl(L"soft_list_v2"));

	for(int index = pList->GetCount() - 1; index >=0; --index) {
		pList->GetItemAt(index)->SetVisible(false);
	}
	
	for (const auto& it : CSoftInfo::GetInstance()->GetSoftInfo()) {
		std::wstring soft_name = LetterHelper::GetLetter(it.m_strSoftName.GetString());
	
		if (!pThis->find_text_.IsEmpty() &&
			nullptr == StrStrIW(soft_name.c_str(), pThis->find_text_.GetData()) &&
			nullptr == StrStrIW(it.m_strSoftName,
								pThis->find_text_)) {
			continue;
		}
	
		pList->Add(pThis->CreateLine(it));
	
		pThis->soft_size_list_.emplace_back(it);
	}
	
	std::thread update_soft_size_thread = std::thread([pThis] {
		for(const auto& it : pThis->soft_size_list_) {
			if(pThis->stop_update_soft_list_) {
				break;
			}

			pThis->UpdateSize(it);
		}

		pThis->soft_size_list_.clear();
	});
	
	pThis->update_soft_size_thread_ = std::move(update_soft_size_thread);

	return 0;
}

void CInstalledWnd::UpdateDate(bool need_update, void* data, size_t update_count) {
	auto info = (UpdateInfo*)data;

	DuiLib::CDuiString name = CStringHelper::a2w(info->name).c_str();

	DuiLib::CListUI* pList = (DuiLib::CListUI*)m_pm.FindControl(L"soft_list_v2");

	int count = pList->GetCount();

	for (int n = 0; n < count; ++n) {
		CSoftListElementUI* line = (CSoftListElementUI*)pList->GetItemAt(n);

		if (nullptr == StrStrIW(line->GetSoftName(), name)) {
			continue;
		}

		if (!info->msg.empty()) {
			line->UpdateMessage(info->msg.c_str());
			continue;
		}

		if(need_update) {
			line->UpdateUpgradeInfo(info->version, info->url);
		}

		break;
	}
}

void CInstalledWnd::ClearData() {
}

void CInstalledWnd::ClearData(void* data) {
	auto info = (UpdateInfo*)data;

	DuiLib::CDuiString name = CStringHelper::a2w(info->name).c_str();

	DuiLib::CListUI* pList = (DuiLib::CListUI*)m_pm.FindControl(L"soft_list_v2");

	int count = pList->GetCount();

	for (int n = 0; n < count; ++n) {
		CSoftListElementUI* line = (CSoftListElementUI*)pList->GetItemAt(n);

		if (nullptr == StrStrIW(line->GetSoftName(), name)) {
			continue;
		}

		if (!info->msg.empty()) {
			line->UpdateMessage(info->msg.c_str());
			break;
		}
	}
}

DWORD CInstalledWnd::OnInstallPackage(WPARAM wParam, LPARAM lParam, LPVOID user_ptr) {
	auto pThis = static_cast<CInstalledWnd*>(user_ptr);

	auto file_path = reinterpret_cast<const char*>(wParam);
	auto url = CStringHelper::a2w(reinterpret_cast<const char*>(lParam));

	const auto pList = dynamic_cast<DuiLib::CListUI*>(pThis->m_pm.FindControl(L"soft_list_v2"));

	auto count = pList->GetCount();

	for (int index = 0; index < count; ++index) {
		auto line = dynamic_cast<CSoftListElementUI*>(pList->GetItemAt(index));

		if (line->GetDownloadUrl() == url) {
			line->InstallPackage(CStringHelper::a2w(file_path).c_str());
			break;
		}
	}

	return 0;
}

LONGLONG CInstalledWnd::GetInstallPathSize(const wchar_t* install_path, const wchar_t* uninstall_path) {
	const auto install_path_temp = FileHelper::GetFilePath(install_path);
	const auto uninstall_path_temp = FileHelper::GetFilePath(uninstall_path);

	if(!install_path_temp.empty()) {
		return Helper::GetDirectorySize(install_path_temp.c_str());
	}

	if (!uninstall_path_temp.empty()) {
		wchar_t szUnInstallPath[MAX_PATH];
		ZeroMemory(szUnInstallPath, MAX_PATH * sizeof(wchar_t));
		wcscpy(szUnInstallPath, uninstall_path_temp.c_str());
		PathRemoveFileSpec(szUnInstallPath);

		if (PathIsDirectory(szUnInstallPath)) {
			return Helper::GetDirectorySize(szUnInstallPath);
		}
	}

	return 0;
}

void CInstalledWnd::UpdateSize(const SoftInfo& info) {
	const DuiLib::CListUI* pList = static_cast<DuiLib::CListUI*>(m_pm.FindControl(L"soft_list_v2"));

	const int count = pList->GetCount();

	for (int n = 0; n < count; ++n) {
		const CSoftListElementUI* line = static_cast<CSoftListElementUI*>(pList->GetItemAt(n));

		if (nullptr == line) {
			break;
		}

		if (line->GetBit() == info.bit
			&& CStringHelper::IsMatch(line->GetSoftName().GetData(), info.m_strSoftName.GetString())) {
			const long long size = GetInstallPathSize(info.m_strInstallLocation.GetString(), info.m_strUninstallPth.GetString());
			line->UpdateSize(Helper::ToWStringSize(size).c_str());
			break;
		}
	}
}

CSoftListElementUI* CInstalledWnd::CreateLine(const SoftInfo& soft_info) const {
	auto line = new CSoftListElementUI;
	line->SetScheme(scheme_.get());
	line->SetSoftName(soft_info.m_strSoftName);
	line->SetBit(soft_info.bit);
	line->SetIcon(soft_info.m_strSoftIcon,
				  soft_info.key_name, 
				  soft_info.m_strInstallLocation);
	line->SetLocalVersion(soft_info.m_strSoftVersion);
	line->SetUninstallPath(soft_info.m_strUninstallPth);

	const std::string soft_name = CStringHelper::w2a(soft_info.m_strSoftName.GetString());

	const auto match_result = UpdateInstance->MatchName(soft_name);
	if(match_result.has_value()) {
		if(match_result->need_update) {
			line->SetUpdateInfo(match_result->version, match_result->url,
								match_result->actions, match_result->type == L"cracked");
		}
		else if(!match_result->msg.empty()) {
			line->SetMessage(match_result->msg.c_str());
		}
	}

	return line;
}
