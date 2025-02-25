#include "main_window.h"

#include <thread>

#include "add_download_task_window.h"
#include "curl_download_manager.h"
#include "curl_download_request.h"
#include "kf_str.h"
#include "scheme.h"
#include "stringHelper.h"

DownloadElementUI::~DownloadElementUI() {
	if (thread_.joinable()) {
		thread_.join();
	}
}

void DownloadElementUI::SetUrl(const char* url) {
	url_ = url;
}

void DownloadElementUI::SetAcceptRanges(bool value) {
	accept_ranges_ = value;
}

void DownloadElementUI::SetSize(double value) {
	size_ = value;
}

void DownloadElementUI::SetSavePath(const wchar_t* value) {
	save_path_ = value;
}

void DownloadElementUI::Init() {
	auto file_name_label = new DuiLib::CLabelUI;

	file_name_label->SetText(PathFindFileName(save_path_.c_str()));

	Add(file_name_label);

	auto rate = new DuiLib::CLabelUI;

	Add(rate);

	Add(new DuiLib::CLabelUI);

	std::thread t([this] {
		CallbackInfo callback_info;

		callback_info.user_ptr = this;
		callback_info.progress_callback = OnProgressCallback;
		callback_info.finished_callback = OnFinishedCallback;
		callback_info.notify_callback = OnNotifyCallback;

		CurlDownloadManager::AddTask(url_.c_str(), accept_ranges_, size_,
									 save_path_.c_str(), callback_info);
	});

	thread_ = std::move(t);
}

int DownloadElementUI::OnProgressCallback(void* ptr, const char* url,
										  double rate) {
	auto pThis = (DownloadElementUI*)ptr;

	pThis->GetItemAt(1)->SetText(std::to_wstring(rate).c_str());

	return 0;
}

void DownloadElementUI::OnFinishedCallback(void* ptr, const char* url,
										   const wchar_t* file_path,
										   CURLcode code, int http_code) {
	auto pThis = (DownloadElementUI*)ptr;

	auto buffer = KfString::Format(L"curl code: %d, http code: %d",
								   code, http_code);

	pThis->GetItemAt(2)->SetText(buffer.GetWString().c_str());
}

void DownloadElementUI::OnNotifyCallback(void* ptr, const char* url,
										 const char* msg) {
	auto pThis = (DownloadElementUI*)ptr;

	pThis->GetItemAt(1)->SetText(CStringHelper::a2w(msg).c_str());
}











LPCTSTR MainWindow::GetSkinFile() {
	return L"main.xml";
}

void MainWindow::Init() {
	scheme_ = std::make_shared<Scheme>();

	scheme_->SetDefaultMode(&m_pm);
}

void MainWindow::NotifyAddTask() {
	auto pFrame = std::make_shared<AddDownloadTaskWindow>();
	pFrame->Create(nullptr, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	pFrame->CenterWindow();
	if (IDOK != pFrame->ShowModal()) {
		return;
	}

	auto url = pFrame->GetUrl();
	auto accept_ranges = pFrame->GetAcceptRanges();
	auto size = pFrame->GetSize();
	auto save_path = pFrame->GetSavePath();

	auto list = (DuiLib::CListUI*)m_pm.FindControl(L"download_list");

	auto element = new DownloadElementUI;

	element->SetSavePath(save_path.c_str());
	element->SetUrl(CStringHelper::w2a(url).c_str());
	element->SetSize(size);
	element->SetAcceptRanges(accept_ranges);

	list->Add(element);
}

void MainWindow::Notify(DuiLib::TNotifyUI& msg) {
	if (msg.sType == DUI_MSGTYPE_CLICK) {
		auto name = msg.pSender->GetName();

		if (name == L"closebtn") {
			Close();
			return;
		}

		if (name == L"minbtn") {
			if(SchemeMode::Dark == scheme_->GetMode()) {
				scheme_->SetLightMode(&m_pm);
			}
			else {
				scheme_->SetDarkMode(&m_pm);
			}

			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
			return;
		}

		if(name == L"btn_add_task") {
			NotifyAddTask();
		}
	}
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if(WM_CLOSE == uMsg) {
		PostQuitMessage(0);
		return 0;
	}

	return CWndImpl::HandleMessage(uMsg, wParam, lParam);
}