#include "installedWnd.h"

#include "event_queue_global_manager.h"
#include "file_helper.h"
#include "helper.h"
#include "installPackageWnd.h"
#include "kf_log.h"
#include "letter_helper.h"
#include "scheme.h"
#include "softInfo.h"
#include "SoftListElementUI.h"
#include "stringHelper.h"
#include "UpdateDetection.h"

CInstalledWnd::~CInstalledWnd() {
	UpdateInstance->GetDataCenter()->Detach(this);
}

void CInstalledWnd::UpdateSoftInfo() {
	HANDLE hThread = CreateThread(nullptr, 0, ThreadUpdateSoftListV2, this, 0, nullptr);
	CloseHandle(hThread);
}

LPCTSTR CInstalledWnd::GetSkinFile() {
	return L"installedWnd.xml";
}

void CInstalledWnd::Init() {
	UpdateInstance->GetDataCenter()->Attach(this);

	HANDLE hThread = CreateThread(nullptr, 0, ThreadUpdateSoftListV2, this, 0, nullptr);
	CloseHandle(hThread);
}

void CInstalledWnd::Notify(DuiLib::TNotifyUI& msg) {
	if(msg.sType == DUI_MSGTYPE_CLICK) {
		wstring strName = msg.pSender->GetName();
		
		if (0 == _wcsicmp(strName.c_str(), L"closebtn")) {
			Close(0);
			return;
		}

		if(0 == _wcsicmp(strName.c_str(), L"search_btn")) {
			soft_size_list_.clear();

			find_text_ = m_pm.FindControl(L"search_edit")->GetText();
			
			HANDLE hThread = CreateThread(nullptr, 0, ThreadUpdateSoftListV2, this, 0, nullptr);
			CloseHandle(hThread);

			return;
		}
	}
}

DWORD CInstalledWnd::ThreadUpdateSoftListV2(LPVOID lParam) {
	CInstalledWnd* pThis = (CInstalledWnd*)lParam;

	DuiLib::CListUI* pList = (DuiLib::CListUI*)pThis->m_pm.FindControl(L"soft_list_v2");

	pList->RemoveAll();
	
	for (const auto& it : CSoftInfo::GetInstance()->GetSoftInfo()) {
		std::wstring soft_name = LetterHelper::GetLetter(it.m_strSoftName.GetString());

		if (!pThis->find_text_.IsEmpty() &&
			nullptr == StrStrIW(soft_name.c_str(), pThis->find_text_.GetData()) &&
			nullptr == StrStrIW(it.m_strSoftName,
								pThis->find_text_)) {
			continue;
		}

		pList->Add(pThis->CreateLine(it));

		pThis->soft_size_list_.emplace_back(it.m_strSoftName, it.bit, it.m_strInstallLocation,
											it.m_strUninstallPth);
	}

	const HANDLE hThread1 = CreateThread(nullptr, 0, ThreadUpdateSoftSize, pThis, 0, nullptr);
	CloseHandle(hThread1);

	return 0;
}

DWORD CInstalledWnd::ThreadUpdateSoftSize(LPVOID lParam) {
	CInstalledWnd* pThis = static_cast<CInstalledWnd*>(lParam);

	while(!pThis->soft_size_list_.empty()) {
		auto begin = pThis->soft_size_list_.begin();

		pThis->UpdateSize(std::get<0>(*begin),
						  std::get<1>(*begin), 
						  std::get<2>(*begin), 
						  std::get<3>(*begin));

		pThis->soft_size_list_.erase(begin);
	}

	return 0;
}

void CInstalledWnd::UpdateDate(bool need_update, void* data) {
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
	}
}

void CInstalledWnd::ClearData() {

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

void CInstalledWnd::UpdateSize(const wchar_t* soft_name, uint8_t bit, const wchar_t* install_path,
							   const wchar_t* uninstall_path) {
	const DuiLib::CListUI* pList = static_cast<DuiLib::CListUI*>(m_pm.FindControl(L"soft_list_v2"));

	const int count = pList->GetCount();

	for (int n = 0; n < count; ++n) {
		const CSoftListElementUI* line = static_cast<CSoftListElementUI*>(pList->GetItemAt(n));

		if (line->GetBit() == bit && CStringHelper::IsMatch(line->GetSoftName().GetData(),
															soft_name)) {
			const long long size = GetInstallPathSize(install_path, uninstall_path);
			line->UpdateSize(CStringHelper::a2w(Helper::ToStringSize(size)).c_str());
			break;
		}
	}
}

CSoftListElementUI* CInstalledWnd::CreateLine(const SoftInfo& soft_info) const {
	std::wstring icon;

	if (soft_info.key_name[0] == '{') {
		icon = FileHelper::GetIconWithGuid(soft_info.key_name);
	}

	if (icon.empty()) {
		icon = FileHelper::GetIconWithExePath(soft_info.m_strSoftIcon);
	}
	else {
		KF_INFO(L"通过guid定位到的软件图标: %s", soft_info.m_strSoftName);
	}

	auto line = new CSoftListElementUI;
	line->SetScheme(scheme_.get());
	line->SetSoftName(soft_info.m_strSoftName);
	line->SetBit(soft_info.bit);
	line->SetIcon(icon.c_str());
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
