#include "main_window.h"

#include <thread>

#include "add_download_task_window.h"
#include "curl_download_request.h"
#include "scheme.h"
#include "stringHelper.h"

LPCTSTR MainWindow::GetSkinFile() {
	return L"main.xml";
}

void MainWindow::Init() {
	scheme_ = std::make_shared<Scheme>();

	scheme_->SetDefaultMode(&m_pm);
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
			auto pFrame = std::make_shared<AddDownloadTaskWindow>();
			pFrame->Create(nullptr, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
			pFrame->CenterWindow();
			pFrame->ShowModal();
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

void MainWindow::OnFinishedCallback(void* ptr, const char* sign, std::wstring_view file_path) {
	const auto pThis = static_cast<MainWindow*>(ptr);

	// if (StrStrIW(file_path.data(), L".exe") || StrStrIW(file_path.data(), L".msi")) {
	// 	pThis->m_pm.FindControl(L"label_download")->SetText(L"开始安装");
	//
	// 	if (Helper::ExecuteApplication(file_path.data(), L"")) {
	// 		EventQueueInstance->SendEvent(EVENT_UPDATE_SOFT_DATA);
	// 	}
	// }
	// else {
	// 	for (auto& it : pThis->actions_) {
	// 		auto name = it[L"name"];
	//
	// 		if (name == L"clipboard") {
	// 			Helper::UpdateClipboard(CStringHelper::w2a(it[L"text"]));
	//
	// 			pThis->m_pm.FindControl(L"label_download")->SetText(L"解压密码已复制到剪切板");
	//
	// 			std::this_thread::sleep_for(std::chrono::seconds(3));
	// 		}
	// 	}
	//
	// 	Helper::OpenFolderAndSelectFile(file_path.data());
	// }
	//
	// pThis->Close();
}

int MainWindow::OnProgressSingleThreadCallback(void* ptr, double total_to_download,
													   double now_downloaded, double, double) {
	auto pThis = static_cast<MainWindow*>(ptr);

	if (pThis->download_request_->IsStop()) {
		return -1;
	}

	if (0.1 > now_downloaded) {
		return 0;
	}

	// if (IsWindow(pThis->m_hWnd)) {
	// 	static DWORD start_time = GetTickCount();
	// 	static long long start_size = 0;
	// 	static long long sub_size = 0;
	//
	// 	const auto now = GetTickCount();
	// 	if (now - start_time >= 1000) {
	// 		sub_size = now_downloaded - start_size;
	// 		start_size = now_downloaded;
	// 		start_time = now;
	// 	}
	//
	// 	double value = now_downloaded * 100.0 / total_to_download;
	//
	// 	auto progress = static_cast<DuiLib::CProgressUI*>(pThis->m_pm.FindControl(L"download_progress"));
	// 	if (nullptr != progress) {
	// 		progress->SetValue(value);
	//
	// 		auto text = KfString::Format("下载进度:%0.2lf%%(%s/%s) %s/s", value,
	// 									 Helper::ToStringSize(now_downloaded).c_str(),
	// 									 Helper::ToStringSize(total_to_download).c_str(),
	// 									 Helper::ToStringSize(sub_size).c_str());
	//
	// 		pThis->m_pm.FindControl(L"label_download")->SetText(text.GetWString().c_str());
	// 	}
	// }

	return 0;
}

int MainWindow::OnProgressFunctionV2(void* ptr, double now_downloaded, double total_to_download,
											 int index, long long now_download_size,
											 long long total_download_size) {
	auto pThis = static_cast<MainWindow*>(ptr);

	if (0.1 > now_downloaded) {
		return 0;
	}

	// if (IsWindow(pThis->m_hWnd)) {
	// 	static DWORD start_time = GetTickCount();
	// 	static long long start_size = 0;
	// 	static long long sub_size = 0;
	//
	// 	const auto now = GetTickCount();
	// 	if (now - start_time >= 1000) {
	// 		sub_size = now_downloaded - start_size;
	// 		start_size = now_downloaded;
	// 		start_time = now;
	// 	}
	//
	// 	auto ver = static_cast<DuiLib::CVerticalLayoutUI*>(pThis->m_pm.FindControl(L"mul_thread_download_layout"));
	// 	auto progress = static_cast<DuiLib::CProgressUI*>(ver->GetItemAt(index - 1));
	// 	if (nullptr != progress) {
	// 		const auto value = now_download_size * 100.0 / total_download_size;
	//
	// 		if (value == 100) {
	// 			if (progress->GetValue() > 50) {
	// 				progress->SetValue(static_cast<int>(value));
	// 			}
	// 		}
	// 		else {
	// 			progress->SetValue(static_cast<int>(value));
	// 		}
	//
	// 		const auto tooltip = KfString::Format("index: %d, now: %s, total: %s, rate: %0.2lf", index,
	// 											  Helper::ToStringSize(now_download_size).c_str(),
	// 											  Helper::ToStringSize(total_download_size).c_str(), value);
	// 		progress->SetToolTip(tooltip.GetWString().c_str());
	// 	}
	//
	// 	double value = now_downloaded * 100.0 / total_to_download;
	//
	// 	progress = (DuiLib::CProgressUI*)pThis->m_pm.FindControl(L"download_progress");
	// 	if (nullptr != progress) {
	// 		progress->SetValue(value);
	//
	// 		auto text = KfString::Format("下载进度:%0.2lf%%(%s/%s) %s/s", value,
	// 									 Helper::ToStringSize(now_downloaded).c_str(),
	// 									 Helper::ToStringSize(total_to_download).c_str(),
	// 									 Helper::ToStringSize(sub_size).c_str());
	//
	// 		pThis->m_pm.FindControl(L"label_download")->SetText(text.GetWString().c_str());
	//
	// 		return 0;
	// 	}
	// }

	return 0;
}

bool MainWindow::DownloadTask(const char* url, const wchar_t* dest_folder) {
	download_request_ = std::make_shared<CurlDownloadRequest>();

	download_request_->SetUrl(url);

	bool accept_ranges = false;
	std::string file_name;
	const auto file_length = download_request_->GetContentLength(&accept_ranges, &file_name);

	wchar_t file_path[MAX_PATH];
	ZeroMemory(file_path, sizeof(wchar_t) * MAX_PATH);
	wcscpy(file_path, dest_folder);
	PathAppend(file_path, CStringHelper::a2w(file_name).c_str());

	download_request_->SetDownloadFinishedCallback(OnFinishedCallback, this, url);

	if (accept_ranges && file_length > 0) {
		const auto count = std::thread::hardware_concurrency();

		download_request_->SetDownloadProgressCallback(OnProgressFunctionV2, this);
		download_request_->DownloadFile(file_length, file_path, count);
		return true;
	}

	return download_request_->DownloadSingleThreadFile(file_path/*, OnProgressSingleThreadCallback, this*/);
}
