#include "installPackageWnd.h"

#include "curl_download_request.h"
#include "define.h"
#include "event_queue_global_manager.h"
#include "helper.h"
#include "kf_str.h"
#include "stringHelper.h"

void CInstallPackageWnd::SetInfo(const wchar_t* url,
								 const std::vector<std::map<std::wstring, std::wstring>>& actions) {
	url_ = CStringHelper::w2a(url);
	actions_ = actions;
}

LPCTSTR CInstallPackageWnd::GetSkinFile() {
	return L"installPackageWnd.xml";
}

void CInstallPackageWnd::Init() {
	HANDLE thread_handle = CreateThread(nullptr, 0, ThreadDelayInit, this, 0, nullptr);
	CloseHandle(thread_handle);
}

void CInstallPackageWnd::NotifyClicked(const wchar_t* name) {
	if(0 == wcscmp(name, L"closebtn")) {
		if (download_request_.get()) {
			download_request_->StopDownload();
		}

		Close(IDCANCEL);
	}
}

void CInstallPackageWnd::Notify(DuiLib::TNotifyUI& msg) {
	if(msg.sType == DUI_MSGTYPE_CLICK) {
		NotifyClicked(msg.pSender->GetName());
	}
}

LRESULT CInstallPackageWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (WM_USER_DOWNLOAD_FAILED_PACKAGE == uMsg) {
		if (IDOK == MessageBox(m_hWnd, TEXT("是否将更新链接复制到剪切板自行下载?"), TEXT("下载失败"),
							   MB_OKCANCEL | MB_ICONQUESTION)) {
			Helper::UpdateClipboard(url_);
		}

		Close(IDCANCEL);
		return 0;
	}

	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CInstallPackageWnd::OnFinishedCallback(void* ptr, std::wstring_view file_path) {
	const auto pThis = static_cast<CInstallPackageWnd*>(ptr);

	if(StrStrIW(file_path.data(), L".exe") || StrStrIW(file_path.data(), L".msi")) {
		pThis->m_pm.FindControl(L"label_download")->SetText(L"开始安装");
		Helper::ExecuteApplication(file_path.data(), L"");

		pThis->Close(IDOK);
		return;
	}

	for (auto& it : pThis->actions_) {
		auto name = it[L"name"];

		if (name == L"clipboard") {
			Helper::UpdateClipboard(CStringHelper::w2a(it[L"text"]));

			pThis->m_pm.FindControl(L"label_download")->SetText(L"解压密码已复制到剪切板");

			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
	}

	Helper::OpenFolderAndSelectFile(file_path.data());

	pThis->Close(IDOK);
}

DWORD CInstallPackageWnd::ThreadDelayInit(LPVOID lParam) {
	auto pThis = static_cast<CInstallPackageWnd*>(lParam);

	if(!pThis->DownLoad(pThis->url_, Helper::GetCacheFile(L"").c_str())) {
		::PostMessage(pThis->m_hWnd, WM_USER_DOWNLOAD_FAILED_PACKAGE, 0, 0);
	}

	return 0;
}

int CInstallPackageWnd::OnProgressMultipleThreadCallback(void* ptr, double now_downloaded,
														 double total_to_download, int index, 
														 long long now_download_size,
														 long long total_download_size) {
	auto pThis = static_cast<CInstallPackageWnd*>(ptr);

	if (0.1 > now_downloaded) {
		return 0;
	}

	if (IsWindow(pThis->m_hWnd)) {
	 	static DWORD start_time = GetTickCount();
		static long long start_size = 0;
		static long long sub_size = 0;

		const auto now = GetTickCount();
		if (now - start_time >= 1000) {
			sub_size = now_downloaded - start_size;
			start_size = now_downloaded;
			start_time = now;
		}

		auto ver = static_cast<DuiLib::CVerticalLayoutUI*>(pThis->m_pm.FindControl(L"mul_thread_download_layout"));
		auto progress = static_cast<DuiLib::CProgressUI*>(ver->GetItemAt(index - 1));
		if (nullptr != progress) {
			const auto value = now_download_size * 100.0 / total_download_size;

			progress->SetValue(static_cast<int>(value));

			const auto tooltip = KfString::Format("index: %d, now: %s, total: %s, rate: %0.2lf", index,
												  Helper::ToStringSize(now_download_size).c_str(),
												  Helper::ToStringSize(total_download_size).c_str(), value);
			progress->SetToolTip(tooltip.GetWString().c_str());
		}

		double value = now_downloaded * 100.0 / total_to_download;

		progress = (DuiLib::CProgressUI*)pThis->m_pm.FindControl(L"download_progress");
		if (nullptr != progress) {
			progress->SetValue(value);

			auto text = KfString::Format("下载进度:%0.2lf%%(%s/%s) %s/s", value,
										 Helper::ToStringSize(now_downloaded).c_str(),
										 Helper::ToStringSize(total_to_download).c_str(),
										 Helper::ToStringSize(sub_size).c_str());

			pThis->m_pm.FindControl(L"label_download")->SetText(text.GetWString().c_str());

			return 0;
		}
	}

	return 0;
}

int CInstallPackageWnd::OnProgressSingleThreadCallback(void* ptr, double total_to_download,
													   double now_downloaded, double speed) {
	const auto pThis = static_cast<CInstallPackageWnd*>(ptr);

	const double value = now_downloaded * 100.0 / total_to_download;

	const auto progress = static_cast<DuiLib::CProgressUI*>(pThis->m_pm.FindControl(L"download_progress"));
	if (nullptr == progress) {
		return 0;
	}

	progress->SetValue(value);

	const auto text = KfString::Format("下载进度:%0.2lf%%(%s/%s) %s/s", value,
									   Helper::ToStringSize(now_downloaded).c_str(),
									   Helper::ToStringSize(total_to_download).c_str(),
									   Helper::ToStringSize(speed).c_str());

	pThis->m_pm.FindControl(L"label_download")->SetText(text.GetWString().c_str());

	return 0;
}

bool CInstallPackageWnd::DownLoad(std::string_view url, std::wstring_view temp_path) {
	download_request_ = std::make_shared<CurlDownloadRequest>();

	download_request_->SetUrl(url.data());

	bool accept_ranges = false;
	std::string file_name;
	const auto file_length = download_request_->GetContentLength(&accept_ranges, &file_name);

	wchar_t file_path[MAX_PATH];
	ZeroMemory(file_path, sizeof(wchar_t) * MAX_PATH);
	wcscpy(file_path, temp_path.data());
	PathAppend(file_path, CStringHelper::a2w(file_name).c_str());

	download_request_->SetDownloadFinishedCallback(OnFinishedCallback, this);

	// 大于1M的文件才切片下载
	if(accept_ranges && file_length > 1024 * 1024) {
		return DownloadMultiThread(file_path, file_length);
	}

	// 单线程下载只要可以正常访问就行
	download_request_->SetDownloadSingleProgressCallback(OnProgressSingleThreadCallback, this);

	return download_request_->DownloadSingleThreadFile(file_path);
}

bool CInstallPackageWnd::DownloadMultiThread(std::wstring_view file_path, double file_length) {
	const auto count = std::thread::hardware_concurrency();

	const auto columns = count / 2;

	auto tile_layout = static_cast<DuiLib::CTileLayoutUI*>(m_pm.FindControl(L"mul_thread_download_layout"));
	tile_layout->SetFixedColumns(columns);

	RECT rect_window;
	GetWindowRect(m_hWnd, &rect_window);

	const auto padding = tile_layout->GetPadding();

	const int item_width = (rect_window.right - rect_window.left - padding.left - padding.left) / columns - 4;

	tile_layout->SetItemSize({ item_width, 8 });
	tile_layout->SetChildPadding(4);
	tile_layout->SetChildVPadding(4);

	for (int n = 0; n < count; ++n) {
		auto progress = new DuiLib::CProgressUI;

		progress->SetBkColor(0xFF999999);
		progress->SetForeImage(L"images\\fore_image.png");

		tile_layout->Add(progress);
	}

	download_request_->SetDownloadProgressCallback(OnProgressMultipleThreadCallback, this);
	download_request_->DownloadFile(file_length, file_path, count);

	return true;
}
