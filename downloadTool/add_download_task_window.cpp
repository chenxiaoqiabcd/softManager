#include "add_download_task_window.h"

#include <thread>

#include "curl_download_request.h"
#include "helper.h"
#include "stringHelper.h"

LPCTSTR AddDownloadTaskWindow::GetSkinFile() {
	return L"add_download_task_window.xml";
}

void AddDownloadTaskWindow::Init() {
	m_pm.FindControl(L"download_info_layout")->SetVisible(false);
	ResizeClient(800, 160);
}

void AddDownloadTaskWindow::NotifyClickOk(DuiLib::TNotifyUI& msg) {
	std::thread get_url_info_thread([this] {
		const auto url = m_pm.FindControl(L"edit_url")->GetText();

		if(url.IsEmpty()) {
			MessageBox(m_hWnd, L"请输入url", nullptr, MB_OK);
			return;
		}

		bool accept_ranges = false;
		std::string file_name;

		CurlDownloadRequest download_request;
		download_request.SetUrl(CStringHelper::w2a(url.GetData()).c_str());
		double content_length = download_request.GetContentLength(&accept_ranges, &file_name);

		m_pm.FindControl(L"edit_download_path")->SetText(CStringHelper::a2w(file_name).c_str());

		m_pm.FindControl(L"download_info_layout")->SetVisible(true);

		m_pm.FindControl(L"edit_size")->SetText(CStringHelper::a2w(Helper::ToStringSize(content_length)).c_str());

		ResizeClient(800, 210);
	});

	get_url_info_thread.detach();
}

void AddDownloadTaskWindow::Notify(DuiLib::TNotifyUI& msg) {
	if(msg.sType == DUI_MSGTYPE_CLICK) {
		const auto& name = msg.pSender->GetName();
		if(0 == name.CompareNoCase(L"closebtn") || 0 == name.CompareNoCase(L"btn_cancel")) {
			Close(IDCANCEL);
			return;
		}

		if(0 == name.CompareNoCase(L"btn_ok")) {
			NotifyClickOk(msg);
		}
	}
}
