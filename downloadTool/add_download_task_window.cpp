#include "add_download_task_window.h"

#include <thread>

#include "curl_download_request.h"
#include "helper.h"
#include "stringHelper.h"

std::wstring AddDownloadTaskWindow::GetUrl() const {
	return m_pm.FindControl(L"edit_url")->GetText().GetData();
}

std::wstring AddDownloadTaskWindow::GetSavePath() const {
	return m_pm.FindControl(L"edit_download_path")->GetText().GetData();
}

bool AddDownloadTaskWindow::GetAcceptRanges() const {
	return accept_ranges_;
}

double AddDownloadTaskWindow::GetSize() const {
	return size_;
}

LPCTSTR AddDownloadTaskWindow::GetSkinFile() {
	return L"add_download_task_window.xml";
}

void AddDownloadTaskWindow::Init() {
	m_pm.FindControl(L"download_info_layout")->SetVisible(false);
	ResizeClient(800, 160);
}

void AddDownloadTaskWindow::NotifyTextChanged(DuiLib::TNotifyUI& msg) {
	std::thread get_url_info_thread([this, msg] {
		const auto url = msg.pSender->GetText();

		if(url.IsEmpty()) {
			return;
		}

		m_pm.FindControl(L"btn_ok")->SetEnabled(false);

		std::wstring file_name;

		CurlDownloadRequest download_request;
		download_request.SetUrl(CStringHelper::w2a(url.GetData()).c_str());
		size_ = download_request.GetContentLength(&accept_ranges_, &file_name);

		wchar_t file_path[MAX_PATH];
		ZeroMemory(file_path, sizeof(file_path));
		GetModuleFileName(nullptr, file_path, MAX_PATH);
		PathRemoveFileSpec(file_path);
		PathAppend(file_path, file_name.c_str());

		m_pm.FindControl(L"edit_download_path")->SetText(file_path);

		m_pm.FindControl(L"download_info_layout")->SetVisible(true);

		auto buffer = Helper::ToWStringSize(size_);

		m_pm.FindControl(L"edit_size")->SetText(buffer.c_str());

		ResizeClient(800, 210);

		m_pm.FindControl(L"btn_ok")->SetEnabled(true);
	});

	get_url_info_thread.detach();
}

void AddDownloadTaskWindow::Notify(DuiLib::TNotifyUI& msg) {
	if(msg.sType == DUI_MSGTYPE_CLICK) {
		const auto& name = msg.pSender->GetName();
		if(0 == name.CompareNoCase(L"closebtn") ||
		   0 == name.CompareNoCase(L"btn_cancel")) {
			Close(IDCANCEL);
			return;
		}

		if(0 == name.CompareNoCase(L"btn_ok")) {
			Close(IDOK);
		}

		return;
	}

	if (msg.sType == DUI_MSGTYPE_TEXTCHANGED) {
		NotifyTextChanged(msg);
	}
}
