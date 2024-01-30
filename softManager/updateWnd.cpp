#include "updateWnd.h"

#include <algorithm>

#include "event_queue_global_manager.h"
#include "file_helper.h"
#include "installPackageWnd.h"
#include "kf_log.h"
#include "kf_str.h"
#include "scheme.h"
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
	auto OnRefreshUpdateWndList = [=](WPARAM wParam, LPARAM lParam) {
		if(!IsWindow(m_hWnd)) {
			return 0;
		}

		const auto soft_list = static_cast<DuiLib::CListUI*>(m_pm.FindControl(L"soft_list"));

		if(nullptr != soft_list) {
			ClearData();
			UpdateDate();
			return 0;
		}

		KF_WARN("soft_list没找到，可以考虑再添加一点延迟");

		return 0;
	};

	
	EventQueueInstance->AppendNewThreadListener(EVENT_REFRESH_UPDATE_WND_SOFT_LIST,
												OnRefreshUpdateWndList);

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

		auto icon = FileHelper::GetIconWithGuid(soft_it.key_name);

		if (icon.empty()) {
			icon = FileHelper::GetIconWithExePath(soft_it.m_strSoftIcon);
		}
		else {
			KF_INFO(L"通过guid定位到的软件图标: %s", soft_it.m_strSoftName);
		}

		auto line = new CUpdateListElementUI;

		line->SetScheme(scheme_.get());
		line->SetSoftName(soft_it.m_strSoftName);
		line->SetIcon(icon.c_str());
		line->SetBit(soft_it.bit);
		line->SetLocalVersion(soft_it.m_strSoftVersion);
		line->SetUpdateInfo(info->version, info->url, info->actions,
							info->type == L"cracked");
		line->SetFixedHeight(40);

		return line;
	}

	return nullptr;
}

void CUpdateWnd::UpdateDate(bool need_update, void* data) {
	if(!need_update) {
		return;
	}

	const auto info = static_cast<UpdateInfo*>(data);

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
